/*!\file window.c
* \brief Scène 3D Kirby avec effets
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
 
typedef struct {
  float pos[3];     
  float vel[3];     
  float color[4];   
  float life;       
  float size;       
  int active;       
} Particle;
 
#define MAX_PARTICLES 100
#define NUM_CLOUDS 1500
 
static struct {
 float x, z;
} _cam = {0.0f, 8.0f};
 
static struct {
  float spinSpeed;      
  float initialSpeed;   
  float spinTime;       
  float spinDuration;   
  int isSpinning;      
} _spinState = {
  8.0f,     /* spinSpeed réduit de 13.0 à 8.0 */
  9.0f,     /* initialSpeed réduit de 15.0 à 9.0 */
  0.0f,     /* spinTime */
  5.0f,     /* spinDuration augmenté de 3.0 à 5.0 pour une rotation plus lente */
  1         /* isSpinning */
};
 
static struct {
  float y;
  float speed;
  float height;
  int direction;
} _kirbyFloat = {
  0.0f,     /* y */
  0.35f,    /* speed */
  0.05f,    /* height */
  1         /* direction */
};
 
static struct {
  float x, y, z;     
  float scale;       
  float rotation;    
  float color[3];    
} _clouds[NUM_CLOUDS];
 
/* -------- VARIABLES GLOBALES -------- */

static GLuint pId[2] = {0};
static GLuint plan = 0;
static GLuint sphere = 0;
static GLuint texId[2] = {0};
static GLuint skyboxTex = 0;
static int ww = 1, wh = 1;
static GLuint quad = 0;
static GLuint _textTexId = 0;
 
static GLuint cloudModel = 0;
static GLuint kirbyModel = 0;
static GLuint starModel = 0;
static GLuint bubbleModel = 0;
 
static float cloudRotationAngle = 0.0f;
static Particle particles[MAX_PARTICLES];
static float elapsedTime = 0.0f;

/* -------- PROTOTYPES -------- */
static void init(void);         // Init scène
static void draw(void);         // Rendu
static void keyd(int code);
static void quit(void);
static void resize(void);
static void resetParticle(Particle *p);  // Init particule
static void updateParticles(float dt);   // Update particules
static void initText(GLuint * ptId, const char * text); //texte
static void updateText(GLuint texId, const char * text); // Met à jour le texte
 
void intro(int state){
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
 
  cloudModel = assimpGenScene("models/clouds/scene.gltf");
  kirbyModel = assimpGenScene("models/kirby/scene.gltf");
  starModel = assimpGenScene("models/star/scene.gltf");
  bubbleModel = assimpGenScene("models/bubble/scene.gltf");
 
  plan = gl4dgGenQuadf();
  sphere = gl4dgGenSpheref(100, 50);
 
  pId[0] = gl4duCreateProgram("<vs>shaders/intro_tex.vs", "<fs>shaders/intro_tex.fs", NULL);
  pId[1] = gl4duCreateProgram("<vs>shaders/intro_texte.vs", "<fs>shaders/intro_texte.fs", NULL);
 
  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "view");
  gl4duGenMatrix(GL_FLOAT, "proj");
  resize();

  quad = gl4dgGenQuadf();
  initText(&_textTexId,"Salut ! C'est Kirby !\nJe suis content que \ntu aies pu venir !");

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
 
  // Init nuages
  for(int i = 0; i < NUM_CLOUDS; i++) {
    _clouds[i].x = gl4dmSURand() * 300.0f - 50.0f;
    _clouds[i].y = gl4dmSURand() * 0.0f - 3.0f;
    _clouds[i].z = gl4dmSURand() * 300.0f - 50.0f;
    _clouds[i].scale = 0.5f + gl4dmURand() * 1.0f;
    _clouds[i].rotation = gl4dmURand() * 360.0f;
    _clouds[i].color[0] = 2.0f;
    _clouds[i].color[1] = 2.0f;
    _clouds[i].color[2] = 2.0f;
  }
 
  // Init particules
  for(int i = 0; i < MAX_PARTICLES; i++) {
    resetParticle(&particles[i]);
  }
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
 

/* -------- GESTION DES PARTICULES -------- */

void resetParticle(Particle *p) {
  // Position autour de Kirby avec un rayon plus petit
  p->pos[0] = gl4dmURand() * 1.5f - 0.8f;  // -0.5 à 0.5 (réduit de -2/2)
  p->pos[1] = 2.0f + gl4dmURand();         // Au-dessus de Kirby
  p->pos[2] = gl4dmURand() * 1.5f - 0.8f;  // -0.5 à 0.5 (réduit de -2/2)
    
  // Vitesse: chute avec mouvement aléatoire
  p->vel[0] = gl4dmURand() * 0.2f - 0.1f;
  p->vel[1] = -0.5f - gl4dmURand() * 0.2f; // Chute
  p->vel[2] = gl4dmURand() * 0.2f - 0.1f;
    
  // Couleur modérée
  p->color[0] = 1.5f;        // R (entre 1.0 et 2.0)
  p->color[1] = 1.45f;       // G (entre 0.97 et 1.94)
  p->color[2] = 0.95f;       // B (entre 0.63 et 1.26)
  p->color[3] = 1.0f;        // A
    
  p->life = 1.5f + gl4dmURand();  // 1-2s de vie
  p->size = 0.07f + gl4dmURand() * 0.05f;
  p->active = 1;
}

void updateParticles(float dt) {
  for(int i = 0; i < MAX_PARTICLES; i++) {
    Particle *p = &particles[i];
    if(!p->active) continue;     
      // Mise à jour position
      p->pos[0] += p->vel[0] * dt;
      p->pos[1] += p->vel[1] * dt;
      p->pos[2] += p->vel[2] * dt;
     
      // Mise à jour vie
      p->life -= dt;
      p->color[3] = p->life; // Disparition
      
      // Reset si morte
      if(p->life <= 0) {
        resetParticle(p);
      }
    }
}

/* -------- RENDU DE LA SCÈNE -------- */

static void resize(void) {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  ww = vp[2];
  wh = vp[3];
  gl4duBindMatrix("proj");
  gl4duLoadIdentityf();
  float ratio = wh / (float)ww;
  gl4duFrustumf(-1, 1, -1 * ratio, 1 * ratio, 1, 1000);
}

void draw(void) {
  resize();
  glEnable(GL_DEPTH_TEST);
  /* Gestion temps */
  static double t0 = 0.0;
  const GLfloat inclinaison = 0.0;
  static double tdepart = -1.0;
  if(tdepart < 0.0) {
    tdepart = gl4dGetElapsedTime() / 1000.0;
  }
  double t = gl4dGetElapsedTime() / 1000.0 - tdepart;
  elapsedTime = t;
  double dt = (t - t0);
  t0 = t;
  elapsedTime = t;  // Met à jour le temps écoulé
  /* Angle incrémental */
  static float a = 0;
  static float totalAngle = 0.0f;
  /* Effacer buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /* Programme GPU */
  glUseProgram(pId[0]);
  glBindTexture(GL_TEXTURE_2D, texId[0]);
  glUniform1i(glGetUniformLocation(pId[0], "tex"), 0);
  
  /* Caméra en rotation */
  float camRadius = 3.0f;  // Distance de la caméra par rapport au centre
  float camHeight = 1.0f;  // Hauteur de la caméra
  
  if (_spinState.isSpinning) {
    _spinState.spinTime += dt;  // Incrémente le temps écoulé depuis le début de la rotation
    if (_spinState.spinTime >= _spinState.spinDuration) {
        // Si la durée de rotation est atteinte, arrête la rotation
        _spinState.isSpinning = 0;
        _spinState.spinSpeed = 0.0f;
        totalAngle = M_PI_2;  // 90 degrés - Position finale fixe
    } else {
        // Calcul de la progression de l'animation (entre 0 et 1)
        float progress = _spinState.spinTime / _spinState.spinDuration;
        float easing;  // Variable pour l'effet de ralentissement
        
        if (progress < 0.8f) {
            // Première phase (80% de l'animation): ralentissement progressif
            easing = 1.0f - (progress * 0.3f);
        } else {
            // Dernière phase (20% de l'animation): ajustement précis vers l'angle cible
            float p = (progress - 0.8f) / 0.2f;  // Normalisation de la progression dans cette phase
            float targetAngle = M_PI_2;  // Cible: 90 degrés
            // Normalise l'angle actuel entre 0 et 2π
            float currentAngleMod = fmodf(totalAngle, 2.0f * M_PI);
            if (currentAngleMod < 0) currentAngleMod += 2.0f * M_PI;
            // Calcule le chemin le plus court vers l'angle cible
            float angleToTarget = targetAngle - currentAngleMod;
            if (angleToTarget > M_PI) angleToTarget -= 2.0f * M_PI;  // Prend le chemin le plus court
            if (angleToTarget < -M_PI) angleToTarget += 2.0f * M_PI;
            // Calcule la vitesse nécessaire pour atteindre l'angle cible
            easing = (1.0f - p) * 0.5f + p * (angleToTarget / (dt * _spinState.initialSpeed));
        }
        
        // Applique le facteur de ralentissement à la vitesse de rotation
        _spinState.spinSpeed = _spinState.initialSpeed * easing;
    }
  }
  
  // Met à jour l'angle total de rotation
  totalAngle += _spinState.spinSpeed * dt;
  
  // Calcule la position de la caméra en coordonnées cartésiennes
  float camX = camRadius * cosf(totalAngle);
  float camZ = camRadius * sinf(totalAngle);
  
  // Configure la matrice de vue pour la caméra
  gl4duBindMatrix("view");
  gl4duLoadIdentityf();
  gl4duLookAtf(camX, camHeight, camZ,  // Position caméra
               0.0f, 0.0f, 0.0f,       // Point ciblé (centre de la scène)
               0.0f, 1.0f, 0.0f);      // Vecteur haut (axe Y)
  
  // Positionne la lumière principale pour Kirby et les étoiles
  const GLfloat Lp[] = {camX * 1.2f, camHeight + 2.0f, camZ * 1.2f, 1.0f};
  const GLfloat LightColor[] = { 1.0f, 0.75f, 0.78f, 1.0f };
  glUniform4fv(glGetUniformLocation(pId[0], "Lp"), 1, Lp);
  glUniform4fv(glGetUniformLocation(pId[0], "LightColor"), 1, LightColor);

  /* CIEL - Premier pour arrière-plan */
  glDepthMask(GL_FALSE); // Désactive l'écriture dans le buffer de profondeur pour le skybox
  glBindTexture(GL_TEXTURE_2D, skyboxTex);
  gl4duBindMatrix("mod");
  gl4duLoadIdentityf();
  gl4duTranslatef(camX, camHeight, camZ); // Déplace le ciel avec la caméra pour donner l'illusion d'infini
  gl4duScalef(10.0f, 10.0f, 10.0f);  // Agrandit la sphère du ciel
  gl4duSendMatrices();
  glUniform1i(glGetUniformLocation(pId[0], "sky"), 1);  // Active le mode "ciel" dans le shader
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);  // Couleur blanche (texture non modifiée)
  gl4dgDraw(sphere);  // Dessine la sphère du ciel
  glUniform1i(glGetUniformLocation(pId[0], "sky"), 0);  // Désactive le mode "ciel"
  glBindTexture(GL_TEXTURE_2D, 0);
  glDepthMask(GL_TRUE); // Réactive l'écriture dans le buffer de profondeur pour les autres objets


  
  /* KIRBY */
  // Animation de flottement vertical pour Kirby
  _kirbyFloat.y += _kirbyFloat.direction * _kirbyFloat.speed * dt;
  if (_kirbyFloat.y > _kirbyFloat.height) {
    // Limite supérieure atteinte, inverse la direction
    _kirbyFloat.y = _kirbyFloat.height;
    _kirbyFloat.direction = -1;
  }
  if (_kirbyFloat.y < -_kirbyFloat.height) {
    // Limite inférieure atteinte, inverse la direction
    _kirbyFloat.y = -_kirbyFloat.height;
    _kirbyFloat.direction = 1;
  }

  
  // Mise à jour et rendu particules
  updateParticles(dt);
  
  // Dessine particules avec éclairage
  for(int i = 0; i < MAX_PARTICLES; i++) {
      Particle *p = &particles[i];
      if(!p->active) continue;
      
      gl4duLoadIdentityf();
      gl4duTranslatef(p->pos[0], p->pos[1], p->pos[2]);
      gl4duScalef(p->size, p->size, p->size);
      gl4duSendMatrices();
      
      // Combine couleur et lumière
      float brightnessFactor = 2.0f; // Luminosité accrue
      glUniform4f(glGetUniformLocation(pId[0], "couleur"), 
                  p->color[0] * brightnessFactor,
                  p->color[1] * brightnessFactor,
                  p->color[2] * brightnessFactor,
                  p->color[3]);
      assimpDrawScene(starModel);
  }
  
  gl4duLoadIdentityf();
  gl4duTranslatef(0.0f, _kirbyFloat.y, 0.0f);  // Mouvement flottant
  gl4duScalef(1.0f, 1.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0, 0.05f, 0.47f, 1.0f);
  assimpDrawScene(kirbyModel);

  /* ÉTOILE (remplace TORE) */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 0.0f, 0.0f, 1.0f);
  gl4duTranslatef(1.0f, 0, 0.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* ÉTOILE 1 */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 0.0f, 0.0f, 1.0f);
  gl4duTranslatef(-1.0f, 0, 0.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* ÉTOILE 2 */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 1.0f, 0.0f, 0.0f);
  gl4duTranslatef(0.0f, 0, -1.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* ÉTOILE 3 */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 1.0f, 0.0f, 0.0f);
  gl4duTranslatef(0.0f, 0, 1.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* ÉTOILE 4 */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 0.0f, 1.0f, 0.0f);
  gl4duTranslatef(1.0f, 0, 0.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* ÉTOILE 5 */
  gl4duLoadIdentityf();
  gl4duRotatef(a*2, 0.0f, 1.0f, 0.0f);
  gl4duTranslatef(-1.0f, 0, 0.0f);
  gl4duScalef(0.2f, 0.3f, 0.4f);
  gl4duRotatef(a*3, 0.0f, 0.0f, 1.0f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 3.0f, 2.91f, 1.89f, 1.0f); // Entre 2.0/1.94/1.26 et 4.0/3.88/2.52
  assimpDrawScene(starModel);

  /* NUAGE (remplace SPHERE) */
  static float speed = 0.05f; // Vitesse
  static float height = 0.05f; // Amplitude
  static float y = 0.0f; // Position
  static int direction = 1; // 1=monte, -1=descend
  double t1 = gl4dGetElapsedTime() / 1000.0; // Temps en secondes
  
  // Mouvement linéaire
  y += direction * speed * dt;
  
  // Inversion à hauteur max/min
  if (y > height) {
    y = height;
    direction = -1;
  }
  if (y < -height) {
    y = -height;
    direction = 1;
  }
  
  gl4duLoadIdentityf();
  gl4duRotatef(90, 0.0f, 1.0f, 0.0f);
  gl4duTranslatef(0.0f, y, 0.0f);
  gl4duTranslatef(0.0f, -0.4f, 0.0f);
  gl4duScalef(1.2f, 0.7f, 0.9f);
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 2.0f, 2.0f, 2.0f, 1.0f); // Luminosité x2
  assimpDrawScene(cloudModel);

  /* CHAMP DE NUAGES */
  cloudRotationAngle += 0.02f * dt; // Rotation lente
  
  for(int i = 0; i < NUM_CLOUDS; i++) {
      gl4duLoadIdentityf();
      
      // Calcul position rotative
      float radius = sqrt(_clouds[i].x * _clouds[i].x + _clouds[i].z * _clouds[i].z);
      float originalAngle = atan2(_clouds[i].z, _clouds[i].x);
      float newAngle = originalAngle + cloudRotationAngle;
      
      float rotatedX = radius * cos(newAngle);
      float rotatedZ = radius * sin(newAngle);
      
      // Déplace avec caméra
      float cloudX = rotatedX + camX;
      float cloudZ = rotatedZ + camZ;
      
      gl4duTranslatef(cloudX, _clouds[i].y, cloudZ);
      gl4duRotatef(_clouds[i].rotation, 0.0f, 1.0f, 0.0f);
      gl4duScalef(3.2f * _clouds[i].scale, 
                  1.8f * _clouds[i].scale, 
                  2.4f * _clouds[i].scale);
      gl4duSendMatrices();
      glUniform4f(glGetUniformLocation(pId[0], "couleur"), 
                  _clouds[i].color[0],
                  _clouds[i].color[1],
                  _clouds[i].color[2], 
                  1.0f);
      assimpDrawScene(cloudModel);
  }

  /* SOL */
  glBindTexture(GL_TEXTURE_2D, texId[1]);
  gl4duLoadIdentityf();
  gl4duTranslatef(0.0f, -20.0f, 0.0f);  // Suit caméra
  gl4duRotatef(-90.0f, 2.0f, 0.0f, 0.0f);
  gl4duScalef(3000.0f, 3000.0f, 3000.0f);  // Plan agrandi
  gl4duSendMatrices();
  glUniform4f(glGetUniformLocation(pId[0], "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
  glUniform2f(glGetUniformLocation(pId[0], "decal"), _cam.x / 2.0f, -_cam.z / 2.0f);
  glUniform1f(glGetUniformLocation(pId[0], "nb_repeat"), 500.0f);
  gl4dgDraw(plan);
  glUniform1f(glGetUniformLocation(pId[0], "nb_repeat"), 1.0f);
  glUniform2f(glGetUniformLocation(pId[0], "decal"), 0.0f, 0.0f);

  /* BULLE DE TEXTE */
  if(elapsedTime >= 7.0) {  // Changé de 3.0 à 8.0 secondes
    glUseProgram(pId[0]); // Assure que le programme GPU principal est utilisé
    const GLfloat BulbleLp[] = {0.0f,0.0f,-1000.0f,1.0f};
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
      gl4duScalef(0.45f, 0.05f, 0.8f);
      gl4duTranslatef(1.3f, 0.0f, -0.6f);
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
    glEnable(GL_DEPTH_TEST);
  }

  /* TEXTE */
  if(elapsedTime >= 7.0) {  // Changé de 3.0 à 8.0 secondes
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(pId[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textTexId);
    glUniform1i(glGetUniformLocation(pId[1], "inv"), 1);
    glUniform1i(glGetUniformLocation(pId[1], "tex"), 0);
    gl4duBindMatrix("mod");
    gl4duLoadIdentityf();
    gl4duScalef(0.45, 0.17, 1.0); // Smaller scale for testing
    gl4duBindMatrix("view");//
    gl4duPushMatrix();//
    gl4duLoadIdentityf();//
    gl4duTranslatef(1.3, 0.55, -2.0f); // Just in front of camera
    gl4duSendMatrices();
    gl4duPopMatrix();//
    gl4dgDraw(quad);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
  if(elapsedTime >= 13.0) {
    updateText(_textTexId, "Je t'invite à une fête.\nEs-tu prêt ?\nALLONS-Y !!!");
  }
  glUseProgram(pId[0]);

  /* Désactive programme GPU */
  glUseProgram(0);
  
  /* Incrémente angle */
  a += 60.0 * dt;
}

/* -------- GESTION DES ENTRÉES -------- */

void keyd(int code) {
  switch(code) {
  case GL4DK_l:
    glBindTexture(GL_TEXTURE_2D, texId[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    break;
  case GL4DK_n:
    glBindTexture(GL_TEXTURE_2D, texId[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
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
  
  /* Libère objets GL4D */
  gl4duClean(GL4DU_ALL);
}
