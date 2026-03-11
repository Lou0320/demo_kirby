#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL4D/gl4du.h>  // Utilisation de GL4Dummies pour les fonctions OpenGL modernes
#include <GL/gl.h>       // OpenGL standard header
#include <GL/glu.h>      // Pour les fonctions GLU (optionnel)
#include "obj_loader.h"  // Inclure le fichier d'en-tête pour la structure et les déclarations des fonctions

Model3D loadOBJ(const char *filename) {
    Model3D model = { 0 };

    // Ouverture du fichier
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return model;  // Retourner un modèle vide
    }

    // Création du VAO, VBO et EBO
    glGenVertexArrays(1, &model.vao);
    if (model.vao == 0) {
        printf("Erreur: Impossible de créer le VAO\n");
        fclose(file);
        return model;
    }
    glBindVertexArray(model.vao);

    glGenBuffers(1, &model.vbo);
    if (model.vbo == 0) {
        printf("Erreur: Impossible de créer le VBO\n");
        fclose(file);
        return model;
    }
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);

    glGenBuffers(1, &model.ebo);
    if (model.ebo == 0) {
        printf("Erreur: Impossible de créer l'EBO\n");
        fclose(file);
        return model;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);

    // Exemple de données de triangle (remplacez ceci par le code de lecture du fichier .obj)
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    GLuint indices[] = { 0, 1, 2 };
    model.indexCount = sizeof(indices) / sizeof(indices[0]);

    // Charger les données dans le VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    if (glGetError() != GL_NO_ERROR) {
        printf("Erreur lors du chargement des données du VBO\n");
        fclose(file);
        return model;
    }

    // Charger les données dans l'EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    if (glGetError() != GL_NO_ERROR) {
        printf("Erreur lors du chargement des données de l'EBO\n");
        fclose(file);
        return model;
    }

    // Définir les attributs de vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    if (glGetError() != GL_NO_ERROR) {
        printf("Erreur lors de la configuration des attributs de vertex\n");
        fclose(file);
        return model;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    fclose(file);
    return model;
}

void drawModel(Model3D model) {
    glBindVertexArray(model.vao);
    glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void freeModel(Model3D *model) {
    glDeleteVertexArrays(1, &model->vao);
    glDeleteBuffers(1, &model->vbo);
    glDeleteBuffers(1, &model->ebo);
}
