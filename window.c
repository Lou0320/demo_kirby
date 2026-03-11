/*!\file window.c
 *
 * \brief Utilisation de GL4Dummies pour réaliser une démo.
 *
 * Ici est géré l'ouverture de la fenêtre ainsi que l'ordonnancement
 * des animations. Apriori les seuls éléments à modifier ici lors de
 * votre intégration sont le tableau static \ref _animations et le nom
 * du fichier audio à lire.
 *
 * \author Farès BELHADJ, amsi@up8.edu
 * \date April 12, 2023
 */

#include <GL4D/gl4du.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4duw_SDL2.h>
#include "animations.h"
#include "audioHelper.h"

/* Prototypes des fonctions statiques contenues dans ce fichier C. */
static void init(void);
static void quit(void);
static void resize(int w, int h);
static void keydown(int keycode);

/*!\brief tableau contenant les animations sous la forme de timeline,
 * ce tableau se termine toujours par l'élémént {0, NULL, NULL,
 * NULL} */
static GL4DHanime _animations[] = {
  { 3000, logo, NULL, NULL },
  {2000, logo, noir, fondu },
  {1000, noir, NULL, NULL },
  {2000, noir, intro, fondu },
  { 13000, intro, NULL, NULL },
  {2000, intro, noir, fondu },
  { 1000, noir, NULL, NULL },
  {2000, noir, transform, fondu },
  { 15000, transform, NULL, NULL },
  {2000, transform, noir, fondu },
  { 1000, noir, NULL, NULL },
  {2000, noir, map1, fondu },
  {7000,map1,NULL,NULL},
  {1000,map1,noir,fondu},
  { 1000, noir, NULL, NULL },
  {2000, noir, car, fondu },
  { 22000, car, NULL, NULL },
  {2000, car, noir, fondu },
  { 1000, noir, NULL, NULL },
  {2000, noir, duck, fondu },
  { 12000, duck, NULL, NULL },
  {1000, duck, zoom, fondu },
  { 6000, zoom, NULL, NULL },
  {2000,zoom,noir,fondu},
  {1000, noir, NULL,NULL},
  { 2000, noir, map2, fondu},
  { 11000, map2, NULL , NULL},
  {2000, map2, noir, fondu},
  { 2000,noir,NULL,NULL},
  { 3000, club, NULL , NULL},
  {2000, club, noir, fondu},
  { 1000,noir,NULL,NULL},
  {2000,noir,dancing,fondu},
  {8000,dancing,NULL,NULL},
  {2000,dancing,noir,fondu},
  {1000,noir,NULL,NULL},
  {2000,noir,dancezoom,fondu},
  {8000,dancezoom,NULL,NULL},
  {2000,dancezoom,noir,fondu},
  {1000,noir,NULL,NULL},
  {2000,noir,credits,fondu},
  { 22000, credits, NULL, NULL },
  {2000, credits, noir, fondu },
  { 1000, noir, NULL, NULL },
  {    0, NULL, NULL, NULL } /* Toujours laisser à la fin */
};

/*!\brief dimensions initiales de la fenêtre */
static GLfloat _dim[] = {960, 540};

/*!\brief fonction principale : initialise la fenêtre, OpenGL, audio
 * et lance la boucle principale (infinie).
 */


int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "It's Party Time !", 
    GL4DW_POS_UNDEFINED, GL4DW_POS_UNDEFINED, 
    _dim[0], _dim[1],
    /* GL4DW_RESIZABLE |  */GL4DW_SHOWN))
    return 1;
  init();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(gl4dhDraw);
  gl4duwKeyDownFunc(keydown);
  ahInitAudio("audios/musique_projet.mp3");
  gl4duwMainLoop();
  return 0;
}

/*!\brief Cette fonction initialise les paramètres et éléments OpenGL
 * ainsi que les divers données et fonctionnalités liées à la gestion
 * des animations.
 */
static void init(void) {
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  gl4dhInit(_animations, _dim[0], _dim[1], animationsInit);
  resize(_dim[0], _dim[1]);
}

/*!\brief paramétre la vue (viewPort) OpenGL en fonction des
 * dimensions de la fenêtre.
 * \param w largeur de la fenêtre.
 * \param w hauteur de la fenêtre.
 */
static void resize(int w, int h) {
  _dim[0] = w; _dim[1] = h;
  glViewport(0, 0, _dim[0], _dim[1]);
}

/*!\brief permet de gérer les évènements clavier-down.
 * \param keycode code de la touche pressée.
 */
static void keydown(int keycode) {
  switch(keycode) {
  case SDLK_ESCAPE:
  case 'q':
    exit(0);
  default: break;
  }
}

/*!\brief appelée à la sortie du programme (atexit).
 */
static void quit(void) {
  ahClean();
  gl4duClean(GL4DU_ALL);
}
