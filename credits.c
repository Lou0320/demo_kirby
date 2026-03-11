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
/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void init(void);
static void resize(void);
static void draw(void);
static void quit(void);
static void initText(GLuint * ptId, const char * text);

void credits(int state){
  switch(state) {
    case GL4DH_INIT: init();    /* initialisation */ return;
    case GL4DH_FREE: quit();    /* libération */ return;
    case GL4DH_UPDATE_WITH_AUDIO: /* update audio */ return;
    default: draw();           /* GL4DH_DRAW */ return;
  }
}
/*!\brief dimensions de la fenêtre */
static int _wW = 1, _wH = 1;
/*!\brief identifiant du programme GLSL */
static GLuint _pId = 0;
/*!\brief identifiant de la géométrie d'un plan */
static GLuint _quad = 0;
/*!\brief identifiant de la texture contenant le texte */
static GLuint _textTexId = 0;
/*!\brief La fonction principale créé la fenêtre d'affichage,
 * initialise GL et les données, affecte les fonctions d'événements et
 * lance la boucle principale d'affichage.*/
/*
int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4Dummies", 0, 0, 
			 _wW, _wH, GL4DW_RESIZABLE | GL4DW_SHOWN))
    return 1;
  init();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}*/
/*!\brief initialise les paramètres OpenGL et les données */
static void init(void) {
  glClearColor(1.0f, 0.7f, 0.7f, 0.0f);
  _pId = gl4duCreateProgram("<vs>shaders/car_texte.vs", "<fs>shaders/car_texte.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "mod");
  gl4duGenMatrix(GL_FLOAT, "proj");
  resize();
  _quad = gl4dgGenQuadf();
  initText(&_textTexId,
    "\n\n\nFin !\n\n\n\n\n\n" 
    "En espérant que ce projet vous ait plu, voici les crédits :\n\n\n\n\n\n\n"

    "Audios :\n\n"

    "Musique de fond : Floating Cat par Michael Grubb, via Uppbeat" 
    "- Licence : FJ8OQSPG3TXWZXRS\n"

    "Musique de fête : Steel Echoes of Night par StockTune -" 
    "Domaine public\n"

    "Son de canard : Pixabay - Licence libre\n\n"

    "Modèles 3D (Sketchfab, licence CC-BY 4.0) :\n\n"

    "Disco Ball par araghon007\n"

    "Bubble Speech par jorge_gauss\n"

    "Karby (Kirby) par Daveytopdog\n"

    "Clouds par farhad.Guli\n"

    "Night Club par MBRODERI2\n"

    "Little Duck par dannzjs\n"

    "Kirby par JadeandPals\n"

    "Star par negar\n"

    "Japan Street par abhayexe\n\n"

    "Images :\n\n"

    "Sol de trèfles : Clover grass par ManyTextures.com - CC-BY-4.0\n"

    "Spirale hypnose : tnrsmr via iStock / Getty Images - Licence" 
    "standard\n"

    "Ciel : Pixabay - Licence libre\n"

    "Carte : Lou-Ann Coquard-Morel\n\n"

    "Avertissement :\n\n"

    "Ce projet est une création non commerciale, réalisée dans le cadre " 
    "d'un concours à portée nationale. Kirby est une marque déposée " 
    "appartenant à Nintendo et HAL Laboratory. " 
    "Ce projet est un hommage de fan, sans affiliation, approbation ou" 
    " parrainage de la part de Nintendo.\n\n\nLou-Ann Coquard-Morel");
}
/*!\brief Cette fonction paramétre la vue (viewport) OpenGL en
 * fonction des dimensions de la fenêtre.*/
static void resize(void) {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  _wW = vp[2];
  _wH = vp[3];
  gl4duBindMatrix("proj");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.1, 0.1, -0.2 * _wH / (GLfloat)_wW, 0.2 * _wH / (GLfloat)_wW, 1, 1000);  // Changed vertical range to -0.2/0.2
}

/*!\brief dessine dans le contexte OpenGL actif. */
static void draw(void) {
  glEnable(GL_DEPTH_TEST);
  const GLfloat inclinaison = -60.0;
  static GLfloat t0 = -1;
  GLfloat t, d;
  if(t0 < 0.0f)
    t0 = SDL_GetTicks();
  t = (SDL_GetTicks() - t0) / 1000.0f, d = -1.8f /* du retard pour commencer en bas */ + 0.25f /* vitesse */ * (t/2);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _textTexId);
  glUniform1i(glGetUniformLocation(_pId, "inv"), 1);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  gl4duBindMatrix("mod");
  gl4duLoadIdentityf();
  gl4duScalef(0.4, 4.0, 1);  // Reduced width (0.4) and increased height (4.0) for taller appearance
  gl4duTranslatef(0, d * cos(inclinaison * M_PI / 180.0f), -2 + d * sin(inclinaison * M_PI / 180.0f));
  gl4duRotatef(inclinaison, 1, 0, 0);
  gl4duSendMatrices();
  gl4dgDraw(_quad);
  glUseProgram(0);
}
/*!\brief appelée au moment de sortir du programme (atexit), libère les éléments utilisés */
static void quit(void) {
  if(_textTexId) {
    glDeleteTextures(1, &_textTexId);
    _textTexId = 0;
  }
  gl4duClean(GL4DU_ALL);
}
/*!\brief création d'une texture avec du texte. 
 *
 * \param ptId pointeur vers la variable stockant l'identifiant de
 * texture OpenGL. Si l'identifiant existe déjà (!= 0) l'utiliser
 * sinon (== 0) en générer un nouveau.
 *
 * \param text pointeur vers la chaine de caractères contenant le
 * texte.
 */
static void initText(GLuint * ptId, const char * text) {
  static int firstTime = 1;
  SDL_Color c = {255, 182, 193, 255};  // Pale pink color
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
