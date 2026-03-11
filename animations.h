/*!\file animations.h
 *
 * \brief Votre espace de liberté : c'est ici que vous pouvez ajouter
 * vos fonctions de transition et d'animation avant de les faire
 * référencées dans le tableau _animations du fichier \ref window.c
 *
 * Des squelettes d'animations et de transitions sont fournis pour
 * comprendre le fonctionnement de la bibliothèque. En bonus des
 * exemples dont un fondu en GLSL.
 *
 * \author Farès BELHADJ, amsi@up8.edu
 * \date April 12, 2023
 */
#ifndef _ANIMATIONS_H

#define _ANIMATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

  extern void fondu(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state);
  extern void fondui(void (* a0)(int), void (* a1)(int), Uint32 t, Uint32 et, int state);
  
  extern void rouge(int state);
  extern void vert(int state);
  extern void bleu(int state);
  extern void noir(int state);
  extern void animationsInit(void);
  extern void intro(int state);
  extern void car(int state);
  extern void transform(int state);
  extern void duck(int state);
  extern void zoom(int state);
  extern void map1(int state);
  extern void map2(int state);
  extern void club(int state);
  extern void dancing(int state);
  extern void dancezoom(int state);
  extern void credits(int state);
  extern void logo(GLuint * ptId, const char * text);
#ifdef __cplusplus
}
#endif

#endif
