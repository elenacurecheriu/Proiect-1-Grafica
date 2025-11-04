#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint
EboId,
VaoId,
VboId,
ColorBufferId,
ProgramId;

GLint myMatrixLocation;
glm::mat4 myMatrix, resizeMatrix;

float xMin = -80.f, xMax = 80.f, yMin = -60.f, yMax = 60.f;

void CreateVAO(void)
{
    static const GLfloat Vertices2[] =
    {
        // acoperis
         30.0f, 40.0f,  0.0f,  1.0f,   // 0
         10.0f, 20.0f,  0.0f,  1.0f,   // 1
         50.0f, 20.0f,  0.0f,  1.0f,   // 2
         // pereti
          15.0f, -5.0f,  0.0f,  1.0f,   // 3
          45.0f, -5.0f,  0.0f,  1.0f,   // 4
    };

    static const GLfloat Colors2[] =
    {
        // acoperis (maro)
        0.6f, 0.3f, 0.0f, 1.0f,
        0.6f, 0.3f, 0.0f, 1.0f,
        0.6f, 0.3f, 0.0f, 1.0f,
        // pereti (gri)
        0.7f, 0.7f, 0.7f, 1.0f,
        0.7f, 0.7f, 0.7f, 1.0f
    };

    static const GLuint Indices2[] =
    {
        0, 1, 2,
        1, 2, 3,
        2, 4, 3
    };

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &ColorBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors2), Colors2, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices2), Indices2, GL_STATIC_DRAW);
}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ColorBufferId);
    glDeleteBuffers(1, &VboId);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("example.vert", "example.frag");
    glUseProgram(ProgramId);

    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
}

void DestroyShaders(void)
{
    glDeleteProgram(ProgramId);
}

void Initialize(void)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    CreateVAO();
    CreateShaders();

    //  inițializăm matricea de proiecție ortogonală
    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);

    //  matricea de transformare 
    // exemplu: mută casa puțin în sus
    myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));

    //  combinăm transformarea cu proiecția
    myMatrix = resizeMatrix * myMatrix;
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    glBindVertexArray(VaoId);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, (void*)(0));

    glFlush();
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBO();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Grafica pe calculator - exemplu cu matrici");

    glewInit();
    Initialize();
    glutDisplayFunc(RenderFunction);
    glutCloseFunc(Cleanup);
    glutMainLoop();
}
