#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <GL/gl.h>

// Structure représentant un modèle 3D
typedef struct {
    GLuint vao;           // Vertex Array Object
    GLuint vbo;           // Vertex Buffer Object
    GLuint ebo;           // Element Buffer Object
    GLsizei indexCount;   // Nombre d'indices pour glDrawElements
} Model3D;

// Fonction pour charger un modèle 3D à partir d'un fichier .obj
Model3D loadOBJ(const char *filename);

// Fonction pour dessiner un modèle 3D
void drawModel(Model3D model);

// Fonction pour libérer la mémoire utilisée par un modèle 3D
void freeModel(Model3D *model);

#endif // OBJ_LOADER_H

