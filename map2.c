/*!\file window.c
* \brief Scène 3D avec ciel et sol
* \author [Lou-Ann]
* \date 2025
*/

#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dm.h>
#include <GL4D/gl4dg.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4df.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <GL4D/gl4dp.h>
#include "obj_loader.h"
#include "assimp.h"

/* -------- STRUCTURES -------- */
 
/* -------- VARIABLES GLOBALES -------- */

static GLuint pId[2] = {0};
static GLuint plan = 0;
static GLuint sphere = 0;
static GLuint texId[2] = {0};
static GLuint skyboxTex = 0;
static int ww = 1, wh = 1;
static GLuint quad = 0;
static GLuint hypnoTex = 0;  
static GLuint hypnoQuad = 0; 

static GLuint streetModel = 0;
static GLuint carbyModel = 0;
static GLuint kirbyModel = 0;

static float elapsedTime = 0.0f;

// Ajout des positions de départ, intermédiaire et d'arrivée
static struct {
    float startX, startY, startZ;
    float midX, midY, midZ;      
    float endX, endY, endZ;
    float duration;              // durée de la transition par segment en secondes
    float startTime;             // temps de début de la transition
    int segment;                 // 0 = start->mid, 1 = mid->end
} carTransition = {  
    -0.1f, 2.3f, -3.0f,         
    0.4f, 1.7f, -3.0f,          
    0.75f, 1.7f, -3.0f,         
    4.0f,                       
    0.0f,                       
    0                           
};

static struct {
  float x, y, z;
} _cam = {0.0f, 2.0f, 8.0f};
 
/* -------- PROTOTYPES -------- */
static void init(void);
static void draw(void);
static void keyd(int code);
static void quit(void);
static void resize(void);

void map2(int state){
  switch(state) {
    case GL4DH_INIT: init();    /* initialisation */ return;
    case GL4DH_FREE: quit();    /* libération */ return;
    case GL4DH_UPDATE_WITH_AUDIO: /* update audio */ return;
    default: draw();           /* GL4DH_DRAW */ return;
  }
}
 
/* -------- MAIN -------- 
 
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "Kirby's party avec GL4Dummies", 20, 20, ww, wh, GL4DW_RESIZABLE | GL4DW_SHOWN))
    return 1;
 
  init();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(draw);
  gl4duwKeyDownFunc(keyd);
  gl4duwMainLoop();
  return 0;
}*/
 
/* -------- INITIALISATION -------- */
 
void init(void) {
  glClearColor(0.75f, 0.85f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Initialiser SDL_image
  if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL_image: %s\n", IMG_GetError());
    exit(1);
  }
  
  plan = gl4dgGenQuadf();
  sphere = gl4dgGenSpheref(100, 50);
  
  streetModel = assimpGenScene("models/street/scene.gltf");
  carbyModel = assimpGenScene("models/car/scene.gltf");
  kirbyModel = assimpGenScene("models/kirby/scene.gltf");
 
  pId[0] = gl4duCreateProgram("<vs>shaders/car_tex.vs", "<fs>shaders/car_tex.fs", NULL);
  pId[1] = gl4duCreateProgram("<vs>shaders/car_texte.vs", "<fs>shaders/car_texte.fs", NULL); // Ajout du shader texte

  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "view");
  gl4duGenMatrix(GL_FLOAT, "proj");
  gl4duBindMatrix("proj");
  gl4duLoadIdentityf();

  resize();
  quad = gl4dgGenQuadf();

  glGenTextures(2, texId);
  glBindTexture(GL_TEXTURE_2D, texId[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  GLuint p[] = { 0, -1, -1, 0 };
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, p);
  glBindTexture(GL_TEXTURE_2D, 0);
 
  glGenTextures(1, &skyboxTex);
  glBindTexture(GL_TEXTURE_2D, skyboxTex);
  SDL_Surface * skySrc = SDL_LoadBMP("images/newsky.bmp");
  if (skySrc) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skySrc->w, skySrc->h, 0, skySrc->format->BytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, skySrc->pixels);
    SDL_FreeSurface(skySrc);
  } else {
    fprintf(stderr, "Échec chargement skybox : %s\n", SDL_GetError());
    GLuint fallback[] = { 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, fallback);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
 
  glBindTexture(GL_TEXTURE_2D, texId[1]);
  SDL_Surface * floorSrc = SDL_LoadBMP("images/clover.bmp");
  if (floorSrc) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, floorSrc->w, floorSrc->h, 0, floorSrc->format->BytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, floorSrc->pixels);
    SDL_FreeSurface(floorSrc);
  } else {
    fprintf(stderr, "Échec chargement sol : %s\n", SDL_GetError());
    GLuint fallback[] = { 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, fallback);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Création de la texture hypno avec l'image spiral.jpg
  glGenTextures(1, &hypnoTex);
  glBindTexture(GL_TEXTURE_2D, hypnoTex);
  
  SDL_Surface* spiralSrc = IMG_Load("images/map.jpg");
  if (spiralSrc) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, spiralSrc->w, spiralSrc->h, 0, 
                 spiralSrc->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA, 
                 GL_UNSIGNED_BYTE, spiralSrc->pixels);
    SDL_FreeSurface(spiralSrc);
  } else {
    fprintf(stderr, "Échec chargement spiral : %s\n", IMG_GetError());
    GLuint fallback[] = { 0xFF0000FF, 0xFF0000FF, 0xFF0000FF, 0xFF0000FF };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
  }
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  hypnoQuad = gl4dgGenQuadf();
}

/* -------- RENDU DE LA SCÈNE -------- */

static void resize(void) {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  ww = vp[2];
  wh = vp[3];
  gl4duBindMatrix("proj");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.3, 0.3, 0, wh / (GLfloat)ww, 1, 1000);  // Réduit de -0.5,0.5 à -0.2,0.2
  //gl4duBindMatrix("modelViewMatrix");
}

void draw(void) {
  resize();
  glEnable(GL_DEPTH_TEST);
  static double tdepart = -1.0;
  if(tdepart < 0.0) {
    tdepart = gl4dGetElapsedTime() / 1000.0;
  }
  double t = gl4dGetElapsedTime() / 1000.0 - tdepart;
  elapsedTime = t;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(pId[0]);
  glBindTexture(GL_TEXTURE_2D, texId[0]);
  glUniform1i(glGetUniformLocation(pId[0], "tex"), 0);
  
  // Position fixe de la caméra
  gl4duBindMatrix("view");
  gl4duLoadIdentityf();
  gl4duLookAtf(0.0f, 0.6f, 1.8f,      // Position de la caméra
               0.0f, 0.6f, -0.8f,     // Point ciblé plus bas (y réduit de 0.3 à -0.2)
               0.0f, 1.0f, 0.0f);       // Vecteur haut normalisé
  gl4duTranslatef(0.0f,-0.1f, 0.3f);
  gl4duBindMatrix("mod");

  // Position fixe de la lumière
  const GLfloat Lp[] = {0.0f, 10.0f, 3.0f, 1.0f};
  const GLfloat LightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glUniform4fv(glGetUniformLocation(pId[0], "Lp"), 1, Lp);
  glUniform4fv(glGetUniformLocation(pId[0], "LightColor"), 1, LightColor);

  /* CIEL */
  glDepthMask(GL_FALSE);
  glBindTexture(GL_TEXTURE_2D, skyboxTex);
  gl4duLoadIdentityf();
  gl4duScalef(50.0f, 50.0f, 50.0f);
  gl4duSendMatrices();
  glUniform1i(glGetUniformLocation(pId[0], "sky"), 1);
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
  gl4dgDraw(sphere);
  glUniform1i(glGetUniformLocation(pId[0], "sky"), 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDepthMask(GL_TRUE);


  /*Voiture kirby*/
  if (t > 2.0)
  {
    if (carTransition.startTime == 0.0f) {
      carTransition.startTime = t;
    }
    
    float timeElapsed = t - carTransition.startTime;
    float progress = timeElapsed / carTransition.duration;
    
    float currentX, currentY, currentZ;
    
    if (progress <= 1.0f) {  
      currentX = carTransition.startX + (carTransition.midX - carTransition.startX) * progress;
      currentY = carTransition.startY + (carTransition.midY - carTransition.startY) * progress;
      currentZ = carTransition.startZ + (carTransition.midZ - carTransition.startZ) * progress;
    } else if (progress <= 2.0f) {
      float segment2Progress = progress - 1.0f;
      currentX = carTransition.midX + (carTransition.endX - carTransition.midX) * segment2Progress;
      currentY = carTransition.midY + (carTransition.endY - carTransition.midY) * segment2Progress;
      currentZ = carTransition.midZ + (carTransition.endZ - carTransition.midZ) * segment2Progress;
    } else {  
      currentX = carTransition.endX;
      currentY = carTransition.endY;
      currentZ = carTransition.endZ;
    }
    
    gl4duLoadIdentityf();
    gl4duTranslatef(currentX, currentY, currentZ);
    gl4duScalef(0.2f, 0.3f, 0.3f);
    gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    if (t<6.0f)
    {
      gl4duRotatef(60.0f, 0.0f, 1.0f, 0.0f);
    }
    else{
      gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    }
    
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
    assimpDrawScene(carbyModel);
  }
  
  

  /* SOL */
  glBindTexture(GL_TEXTURE_2D, texId[1]);
  gl4duLoadIdentityf();
  gl4duTranslatef(0.0f, -3.0f, 0.0f);  // Sol à y=0
  gl4duRotatef(-90.0f, 2.0f, 0.0f, 0.0f);
  gl4duScalef(3000.0f, 3000.0f, 3000.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
  glUniform2f(glGetUniformLocation(pId[0], "decal"), 0.0f, 0.0f);
  glUniform1f(glGetUniformLocation(pId[0], "nb_repeat"), 500.0f);
  gl4dgDraw(plan);
  glUniform1f(glGetUniformLocation(pId[0], "nb_repeat"), 1.0f);
  glUniform2f(glGetUniformLocation(pId[0], "decal"), 0.0f, 0.0f);

  /* Affichage du quad de la map */
  glUseProgram(pId[1]); 
  glBindTexture(GL_TEXTURE_2D, hypnoTex);
  gl4duLoadIdentityf();
  gl4duTranslatef(0.0f, 2.5f, -5.0f);  
  gl4duScalef(2.0f, 2.0f, 2.0f);
  gl4duRotatef(180.0f, 0.0f, 1.0f, 0.0f);
  gl4duRotatef(180.0f, 0.0f, 0.0f, 1.0f);
  
  gl4duSendMatrices();
  glUniform1i(glGetUniformLocation(pId[1], "tex"), 0); // Ajoute l'uniform pour la texture
  glUniform1i(glGetUniformLocation(pId[1], "inv"), 0); // Désactive l'inversion des couleurs
  gl4dgDraw(hypnoQuad);

  glUseProgram(pId[0]); // Retourne au shader principal
  glUseProgram(0);
}

/* -------- GESTION DES ENTRÉES -------- */
void keyd(int code) {
  switch(code) {
  case GL4DK_ESCAPE:
    exit(0);
    break;
  default:
    break;
  }
}

/* -------- NETTOYAGE -------- */

/* Appelé au exit */
void quit(void) {
  if(texId[0] != 0) {
    glDeleteTextures(2, texId);
    texId[0] = 0;
  }
  
  if (skyboxTex != 0) {
    glDeleteTextures(1, &skyboxTex);
    skyboxTex = 0;
  }

  if(hypnoTex != 0) {
    glDeleteTextures(1, &hypnoTex);
    hypnoTex = 0;
  }

  IMG_Quit();
  
  /* Libère objets GL4D */
  gl4duClean(GL4DU_ALL);
}
