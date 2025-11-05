#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <cmath>

GLuint vaoBg, vboBg, colorBg;
GLuint ProgramId;
GLint myMatrixLocation, colorFactorLocation;
glm::mat4 myMatrix, resizeMatrix;
float xMin = -100.f, xMax = 100.f, yMin = -100.f, yMax = 100.f;
int windowWidth = 1200, windowHeight = 800;

// se adauga numSegments de triunghiuri care se invart in jurul unui punct ca sa formeze un cerc
// daca fac numSegments mai mic de exemplu 10 voi avea un poligon cu 10 laturi in loc de un cerc
int numSegments = 40;

int treeStartIndex = 8; // indexul de start pentru desenarea copacilor in VAO
int treeEndIndex = treeStartIndex + 6 * (numSegments + 2); // 6 cercuri pentru coroana copacului

int cloudStartIndex = treeEndIndex; // indexul de start pentru desenarea norilor in VAO
int cloudEndIndex = cloudStartIndex + 4 * (numSegments + 2);

void CreateVAO() {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;

	// ====== Solul ======
    GLfloat groundVertices[] = {
    -windowWidth, -75.0f, 0.0f, 1.0f,
     windowWidth, -75.0f, 0.0f, 1.0f,
     windowWidth, -100.0f, 0.0f, 1.0f,
    -windowWidth, -100.0f, 0.0f, 1.0f
    };

    GLfloat groundColors[] = {
        0.36f, 0.25f, 0.20f, 1.0f, // maro 
        0.36f, 0.25f, 0.20f, 1.0f,
        0.36f, 0.25f, 0.20f, 1.0f,
        0.36f, 0.25f, 0.20f, 1.0f
    };

    vertices.insert(vertices.end(), groundVertices, groundVertices + 16);
    colors.insert(colors.end(), groundColors, groundColors + 16);

    // ====== Trunchiul ======
    GLfloat trunkVertices[] = {
        -7.0f, -40.0f, 0.0f, 1.0f,
         7.0f, -40.0f, 0.0f, 1.0f,
         7.0f,  20.0f, 0.0f, 1.0f,
        -7.0f,  20.0f, 0.0f, 1.0f
    };

    GLfloat trunkColors[] = {
        0.55f, 0.27f, 0.07f, 1.0f,
        0.55f, 0.27f, 0.07f, 1.0f,
        0.55f, 0.27f, 0.07f, 1.0f,
        0.55f, 0.27f, 0.07f, 1.0f
    };

    vertices.insert(vertices.end(), trunkVertices, trunkVertices + 16);
    colors.insert(colors.end(), trunkColors, trunkColors + 16);

    // ====== Cercurile (pentru copaci si nori) ======

	//                   coordonate centru cerc,   raza,    culoare
    auto addCircle = [&](float cx, float cy, float radius, glm::vec4 color) {
        // centru
        vertices.insert(vertices.end(), { cx, cy, 0.0f, 1.0f });
        colors.insert(colors.end(), { color.r, color.g, color.b, 1.0f });
        // punctele periferice
        // ne invartim in jurul cercului si adaugam toate punctele triunghiurilor
        for (int i = 0; i <= numSegments; i++) {
            float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
            float x = radius * cos(theta);
            float y = radius * sin(theta);
            vertices.insert(vertices.end(), { cx + x, cy + y, 0.0f, 1.0f });
            colors.insert(colors.end(), { color.r, color.g, color.b, 1.0f });
        }
        };

    // coordonatele si culorile cercurile din coroana
    addCircle(0.0f, 45.0f, 25.0f, glm::vec4(110.0f / 255.0f, 181.0f / 255.0f, 43.0f / 255.0f, 1.0f)); // cerc sus centru
    addCircle(-15.0f, 40.0f, 22.0f, glm::vec4(110.0f / 255.0f, 181.0f / 255.0f, 43.0f / 255.0f, 1.0f));   // cerc sus stanga
    addCircle(15.0f, 40.0f, 22.0f, glm::vec4(110.0f / 255.0f, 181.0f / 255.0f, 43.0f / 255.0f, 1.0f));   // cerc sus dreapta
    addCircle(0.0f, 35.0f, 25.0f, glm::vec4(87.0f / 255.0f, 140.0f / 255.0f, 24.0f / 255.0f, 1.0f)); // cerc jos centru
    addCircle(-20.0f, 30.0f, 22.0f, glm::vec4(87.0f / 255.0f, 140.0f / 255.0f, 24.0f / 255.0f, 1.0f)); // cerc jos stanga
    addCircle(20.0f, 30.0f, 22.0f, glm::vec4(87.0f / 255.0f, 140.0f / 255.0f, 24.0f / 255.0f, 1.0f));  // cerc jos dreapta

    // coordonatele norilor
	addCircle(0.0f, 5.0f, 10.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    addCircle(-10.0f, 0.0f, 10.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    addCircle(0.0f, 0.0f, 10.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    addCircle(10.0f, 0.0f, 10.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); 


    // ====== Trimitem toate datele intr-un singur VAO ======
    glGenVertexArrays(1, &vaoBg);
    glBindVertexArray(vaoBg);

    // VBO pentru coordonate
    glGenBuffers(1, &vboBg);
    glBindBuffer(GL_ARRAY_BUFFER, vboBg);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // VBO pentru culori
    glGenBuffers(1, &colorBg);
    glBindBuffer(GL_ARRAY_BUFFER, colorBg);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void CreateShaders() {
    ProgramId = LoadShaders("example.vert", "example.frag");
    glUseProgram(ProgramId);
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    colorFactorLocation = glGetUniformLocation(ProgramId, "colorFactor");
}

void Initialize() {
    glClearColor(0.7f, 0.9f, 1.0f, 1.0f); // cer albastru deschis
    CreateVAO();
    CreateShaders();

    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
    myMatrix = resizeMatrix;
}

void DrawTree(glm::vec2 position, float scale = 1.0f, float colorFactor = 1.0f) {
    glBindVertexArray(vaoBg);

    myMatrix = glm::translate(resizeMatrix, glm::vec3(position, 0.0f));
    myMatrix = glm::scale(myMatrix, glm::vec3(scale, scale, 1.0f));

    glUniform1f(colorFactorLocation, colorFactor);
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    // desenam partile copacului
    glDrawArrays(GL_QUADS, 4, 4);         // trunchi
    for (int i = treeStartIndex; i < treeEndIndex; i = i + numSegments + 2) // pentru fiecare cerc din coroana
        glDrawArrays(GL_TRIANGLE_FAN, i, 42);

}

void DrawCloud(glm::vec2 position, float scale = 1.0f) {
    glBindVertexArray(vaoBg);

    myMatrix = glm::translate(resizeMatrix, glm::vec3(position, 0.0f));
    myMatrix = glm::scale(myMatrix, glm::vec3(scale, scale, 1.0f));

    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    // desenam norii
    for( int i = cloudStartIndex; i < cloudEndIndex; i = i + numSegments + 2)
        glDrawArrays(GL_TRIANGLE_FAN, i, 42);

}

void RenderFunction() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vaoBg);

    // --- SOL ---
    glUniform1f(colorFactorLocation, 1.0f);
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &resizeMatrix[0][0]);
    glDrawArrays(GL_QUADS, 0, 4);

    // === COPACII ===
    // randul
    DrawTree(glm::vec2(-150, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-130, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-90, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-50, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-10, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(30, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(70, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(110, -65), 0.3f, 0.8f);    
    DrawTree(glm::vec2(130, -65), 0.3f, 0.8f);


    // randul de mijloc    
    DrawTree(glm::vec2(-180, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(-160, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(-120, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(-80, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(-40, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(20, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(40, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(80, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(120, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(160, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(180, -65), 0.4f, 0.9f);

    // randul cel mai apropiat
    DrawTree(glm::vec2(-170, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(-150, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(-100, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(-50, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(0, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(50, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(100, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(150, -65), 0.5f, 1.0f);
    DrawTree(glm::vec2(170, -65), 0.5f, 1.0f);

    // === NORII ===
    glUniform1f(colorFactorLocation, 1.0f); 

    DrawCloud(glm::vec2(-150, 80), 0.8f);
    DrawCloud(glm::vec2(-120, 70), 0.8f);
    DrawCloud(glm::vec2(-80, 80), 0.8f);
    DrawCloud(glm::vec2(-30, 70), 0.9f);
    DrawCloud(glm::vec2(30, 75), 1.0f);
    DrawCloud(glm::vec2(90, 80), 0.8f);
    DrawCloud(glm::vec2(130, 85), 0.7f);
    DrawCloud(glm::vec2(160, 75), 0.7f);

    glFlush();
}


void Cleanup() {
    glDeleteProgram(ProgramId);
    glDeleteBuffers(1, &vboBg);
    glDeleteBuffers(1, &colorBg);
    glDeleteVertexArrays(1, &vaoBg);
}

void ReshapeFunction(int width, int height)
{
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;

    // inaltimea ramane la fel dar se modifica latimea in functie de raportul de mai sus
    float viewHeight = yMax - yMin;
    float viewWidth = viewHeight * aspect;

    float centerX = (xMax + xMin) / 2.0f;
    float centerY = (yMax + yMin) / 2.0f;

    float halfWidth = viewWidth / 2.0f;
    float halfHeight = viewHeight / 2.0f;

	// pentru ca formele sa ramana la aceeasi marime indiferent de dimensiunea ferestrei
    resizeMatrix = glm::ortho(centerX - halfWidth, centerX + halfWidth,
        centerY - halfHeight, centerY + halfHeight);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Proiect 1");
    glewInit();
    Initialize();
    glutDisplayFunc(RenderFunction);
    glutReshapeFunc(ReshapeFunction);
    glutCloseFunc(Cleanup);
    glutMainLoop();
}
