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
static GLuint _textTexId = 0;
static GLuint hypnoTex = 0;  // Nouvelle texture pour l'hypnose
static GLuint hypnoQuad = 0; // Nouveau quad pour l'hypnose

static GLuint streetModel = 0;
static GLuint bubbleModel = 0;
static GLuint carbyModel = 0;
static GLuint kirbyModel = 0;  // Nouveau modèle Kirby

static float elapsedTime = 0.0f;

static struct {
  float x, y, z;
} _cam = {0.0f, 2.0f, 8.0f};

static struct {
  float scale_x;
  float scale_y;
  float pos_x;
  float pos_y;
} _textPos = {
  0.23f,   /* scale_x initial */
  0.17f,   /* scale_y initial */
  2.8f,    /* pos_x initial */
  2.9f     /* pos_y initial */
};
 
/* -------- PROTOTYPES -------- */
static void init(void);
static void draw(void);
static void keyd(int code);
static void quit(void);
static void resize(void);
static void initText(GLuint * ptId, const char * text); //texte
static void updateText(GLuint texId, const char * text); // Met à jour le texte
static void updateTextPosition(void); // Met à jour la position du texte
 
void transform(int state){
  switch(state) {
    case GL4DH_INIT: init();    /* initialisation */ return;
    case GL4DH_FREE: quit();    /* libération */ return;
    case GL4DH_UPDATE_WITH_AUDIO: /* update audio */ return;
    default: draw();           /* GL4DH_DRAW */ return;
  }
}

/* -------- MAIN -------- */
/*
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
}
*/
/* -------- INITIALISATION -------- */
 
void init(void) {
  glClearColor(0.75f, 0.85f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Initialiser SDL_image
  if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
    fprintf(stderr, "Erreur lors de l'initialisation de SDL_image: %s\n", IMG_GetError());
    exit(1);
  }
 
  bubbleModel = assimpGenScene("models/bubble/scene.gltf");
  plan = gl4dgGenQuadf();
  sphere = gl4dgGenSpheref(100, 50);
  
  streetModel = assimpGenScene("models/street/scene.gltf");
  carbyModel = assimpGenScene("models/car/scene.gltf");
  kirbyModel = assimpGenScene("models/kirby/scene.gltf");  // Chargement du modèle Kirby
 
  pId[0] = gl4duCreateProgram("<vs>shaders/car_tex.vs", "<fs>shaders/car_tex.fs", NULL);
  pId[1] = gl4duCreateProgram("<vs>shaders/car_texte.vs", "<fs>shaders/car_texte.fs", NULL);

  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "view");
  gl4duGenMatrix(GL_FLOAT, "proj");
  gl4duBindMatrix("proj");

  
  resize();
  quad = gl4dgGenQuadf();
  initText(&_textTexId,"C'est l'heure\nde la...\nTRANSFORMATION !");

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
  
  SDL_Surface* spiralSrc = IMG_Load("images/spiral.jpg");
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

static void initText(GLuint * ptId, const char * text) {
  static int firstTime = 1;
  SDL_Color c = {0, 0, 0, 255}; // Changé de {255, 255, 0, 255} à {0, 0, 0, 255} pour du noir
  SDL_Surface * d, * s;
  TTF_Font * font = NULL;
  if(firstTime) {
    /* initialisation de la bibliothèque SDL2 ttf */
    if(TTF_Init() == -1) {
      fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
      exit(2);
    }
    firstTime = 0;
  }
  if(*ptId == 0) {
    /* initialisation de la texture côté OpenGL */
    glGenTextures(1, ptId);
    glBindTexture(GL_TEXTURE_2D, *ptId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  /* chargement de la font */
  if( !(font = TTF_OpenFont("DejaVuSans-Bold.ttf", 128)) ) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  /* création d'une surface SDL avec le texte */
  d = TTF_RenderUTF8_Blended_Wrapped(font, text, c, 2048);
  if(d == NULL) {
    TTF_CloseFont(font);
    fprintf(stderr, "Erreur lors du TTF_RenderText\n");
    return;
  }
  /* copie de la surface SDL vers une seconde aux spécifications qui correspondent au format OpenGL */
  s = SDL_CreateRGBSurface(0, d->w, d->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  assert(s);
  SDL_BlitSurface(d, NULL, s, NULL);
  SDL_FreeSurface(d);
  /* transfert vers la texture OpenGL */
  glBindTexture(GL_TEXTURE_2D, *ptId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
  fprintf(stderr, "Dimensions de la texture : %d %d\n", s->w, s->h);
  SDL_FreeSurface(s);
  TTF_CloseFont(font);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void updateText(GLuint texId, const char * text) {
  SDL_Color c = {0, 0, 0, 255};
  SDL_Surface * d, * s;
  TTF_Font * font = NULL;

  if(!(font = TTF_OpenFont("DejaVuSans-Bold.ttf", 128))) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }

  d = TTF_RenderUTF8_Blended_Wrapped(font, text, c, 2048);
  if(d == NULL) {
    TTF_CloseFont(font);
    fprintf(stderr, "Erreur lors du TTF_RenderText\n");
    return;
  }

  s = SDL_CreateRGBSurface(0, d->w, d->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  assert(s);
  SDL_BlitSurface(d, NULL, s, NULL);
  SDL_FreeSurface(d);

  glBindTexture(GL_TEXTURE_2D, texId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
  
  SDL_FreeSurface(s);
  TTF_CloseFont(font);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void updateTextPosition(void) {
  float baseRatio = 960.0f / 540.0f;  // Ratio de référence
  float currentRatio = (float)ww / (float)wh;
  float ratioFactor = baseRatio / currentRatio;
  
  _textPos.scale_x = 0.23f * ratioFactor;
  _textPos.scale_y = 0.17f * ratioFactor;
  _textPos.pos_x = 2.8f * ratioFactor;
  _textPos.pos_y = 2.9f;
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
  updateTextPosition();
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
               0.0f, -5.0f, -12.0f,     // Point ciblé plus bas (y réduit de 0.3 à -0.2)
               0.0f, 1.0f, 0.0f);       // Vecteur haut normalisé
  gl4duTranslatef(0.0f, -0.8f, 0.3f);
  gl4duRotatef(-12.0f,1.0f,0.0f, 0.0f);
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

  /* Voiture kirby et Kirby */
  static float newX = 0.0f;
  float carZ = -0.6f * t;
  float rotationSpeed;
  float currentRotation;

  if(t <= 5.0f) {
    // Rotation beaucoup plus rapide pour Kirby
    rotationSpeed = 720.0f * (t / 5.0f);
    currentRotation = rotationSpeed * t;
    
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0f, 1.5f, -3.0);
    gl4duRotatef(currentRotation, 0.0f, 1.0f, 0.0f);
    gl4duScalef(0.4f, 0.6f, 0.4f);
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
    assimpDrawScene(kirbyModel);
  } else {
    // Ralentissement plus rapide pour la voiture
    float transitionTime = (t - 5.0f);
    rotationSpeed = 720.0f * exp(-transitionTime * 3.0f);
    currentRotation = rotationSpeed * t;
    
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0f, 1.5f, -3.0);
    gl4duRotatef(currentRotation, 0.0f, 1.0f, 0.0f);
    gl4duScalef(0.65f, 0.95f, 0.65f);
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

  /* Affichage du quad hypno */
  glUseProgram(pId[1]);
  glBindTexture(GL_TEXTURE_2D, hypnoTex);
  gl4duLoadIdentityf();
  gl4duTranslatef(0.0f, 2.0f, -5.0f);  
  gl4duScalef(3.0f, 3.0f, 3.0f);
  gl4duRotatef(100.0f * t, 0.0f, 0.0f, 1.0f);  // Rotation sur Z
  
  gl4duSendMatrices();
  glUniform1i(glGetUniformLocation(pId[1], "tex"), 0); // Ajoute l'uniform pour la texture
  glUniform1i(glGetUniformLocation(pId[1], "inv"), 0); // Désactive l'inversion des couleurs
  gl4dgDraw(hypnoQuad);

  glUseProgram(pId[0]); // Retourne au shader principal
  glUseProgram(0);

  /*Bulle de texte*/
  // Position de lumière spécifique pour la bulle
  if(elapsedTime >= 2.0) {  // Changé de 3.0 à 8.0 secondes
    glEnable(GL_DEPTH_TEST);
    glUseProgram(pId[0]); // Assure que le programme GPU principal est utilisé
    const GLfloat BulbleLp[] = {0.0f,0.0f,-1000.0f,1.0f};//-_cam.x * 1.2f, _cam.y + 2.0f, -_cam.z * 1.2f, 1.0f};
    glUniform4fv(glGetUniformLocation(pId[0], "Lp"), 1, BulbleLp);
    // Dessine la bulle
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Définit une couleur blanche moins lumineuse pour la bulle
    glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 3.0f, 3.0f, 1.0f); // Entre 2.0 et 4.0

    gl4duBindMatrix("view");
    gl4duLoadIdentityf();
    // Position fixe de la bulle sur l'écran
    gl4duBindMatrix("proj");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duBindMatrix("mod");
      gl4duLoadIdentityf();
      gl4duRotatef(90, 1.0f, 0.0f, 0.0f);
      gl4duTranslatef(0.585f, -0.8f, -0.48f);
      gl4duScalef(0.45f, 0.05f, 0.8f);
      gl4duSendMatrices();
      // Dessine la bulle
      assimpDrawScene(bubbleModel);
    }
    gl4duBindMatrix("proj");
    gl4duPopMatrix();
    gl4duBindMatrix("mod");
    glDisable(GL_BLEND);
    // Restaure la lumière principale avant de continuer
    glUniform4fv(glGetUniformLocation(pId[0], "Lp"), 1, Lp);
  }

  /* TEXTE */
  if(elapsedTime >= 2.0) {  // Changé de 3.0 à 8.0 secondes
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(pId[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textTexId);
    glUniform1i(glGetUniformLocation(pId[1], "inv"), 1);
    glUniform1i(glGetUniformLocation(pId[1], "tex"), 0);
    gl4duBindMatrix("view");
    gl4duLoadIdentityf();
    gl4duBindMatrix("proj");
    gl4duPushMatrix(); {
      gl4duLoadIdentityf();
      gl4duBindMatrix("mod");
      gl4duLoadIdentityf();
      gl4duScalef(_textPos.scale_x, _textPos.scale_y, 1.0); // Utilise les échelles calculées
      gl4duTranslatef(_textPos.pos_x, _textPos.pos_y, 0.0f); // Utilise les positions calculées
      gl4duSendMatrices();
    }
    gl4duBindMatrix("proj");
    gl4duPopMatrix();
    gl4duBindMatrix("mod");
    gl4dgDraw(quad);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
  if(elapsedTime >= 10.0f) {
    updateText(_textTexId, "Tu n'avais tout\nde même pas cru\nqu'on allait marcher ?");
  }
  if(elapsedTime >= 14.0) {
    updateText(_textTexId, "En voiture !\nNous allons\nrécupérer un ami !");
  }
  glUseProgram(pId[0]);

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

  if(_textTexId) {
    glDeleteTextures(1, &_textTexId);
    _textTexId = 0;
  }

  if(hypnoTex != 0) {
    glDeleteTextures(1, &hypnoTex);
    hypnoTex = 0;
  }

  IMG_Quit(); // Ajout du nettoyage de SDL_image
  
  /* Libère objets GL4D */
  gl4duClean(GL4DU_ALL);
}
