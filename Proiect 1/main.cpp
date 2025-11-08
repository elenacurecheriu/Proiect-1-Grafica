#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <cmath>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

GLuint vaoBg, vboBg, colorBg;
GLuint vaoBird, vboBird, colorBird;
GLuint ProgramId;
GLint myMatrixLocation, colorFactorLocation;
glm::mat4 myMatrix, resizeMatrix;
float xMin = -100.f, xMax = 100.f, yMin = -100.f, yMax = 100.f;
int windowWidth = 1200, windowHeight = 800;

// Limitele dinamice ale Vao ului Curent (folosite pentru maparea mouse ului)
float vao_xMin_current = xMin;
float vao_xMax_current = xMax;
float vao_yMin_current = yMin;
float vao_yMax_current = yMax;

struct BirdData {
    float currentX;
    float currentY;

    float lerpFactor;
    glm::vec2 permanentOffset;

    float scale;
    float wingTimeOffset;
    float wingAmplitude;
    float wingSpeedFactor;
};

std::vector<BirdData> stol;
float birdOffset = 0.0f; // pozitia pasarii pe axa x
float birdSpeed = 0.2f; // viteza de deplasare a pasarii
float wingTime = 0.0f; // timpul pentru animatia aripilor
bool isMouseTracking = false;
const float autoFlySpeed = 0.05f;

// pentru efectul de interschimbare
float swapTimer = 0.0f;
const float swapInterval = 3.0f;
const float swapDuration = 1.0f; // durata animației de swap
bool isSwapping = false;

struct SpawnedBird 
{
    float xOffset;
    float yPosition;
    float targetX; // pozitia finala dupa interschimbare
    float targetY;
    float startX;  // poziția de start pentru swap
    float startY;  // poziția de start pentru swap
    float swapTimer; // timer individual pentru fiecare pasăre
    float scale;
    float speed;
    float wingPhase;
    bool isOnScreen;
    bool isSwapping;
};

vector <SpawnedBird> spawnedBirds; //pasarile pe care le adaugam la apasarea tastei space

// se adauga numSegments de triunghiuri care se invart in jurul unui punct ca sa formeze un cerc
// daca fac numSegments mai mic de exemplu 10 voi avea un poligon cu 10 laturi in loc de un cerc
int numSegments = 40;

int treeStartIndex = 8; // indexul de start pentru desenarea copacilor in VAO
int treeEndIndex = treeStartIndex + 6 * (numSegments + 2); // 6 cercuri pentru coroana copacului

int cloudStartIndex = treeEndIndex; // indexul de start pentru desenarea norilor in VAO
int cloudEndIndex = cloudStartIndex + 4 * (numSegments + 2);


void CreateBirdVAO() {
    GLfloat birdVertices[] = {
        // aripa stanga
        0.0f, 0.0f, 0.0f, 1.0f,
        -3.0f, 2.0f, 0.0f, 1.0f,
        // aripa dreapta
        0.0f, 0.0f, 0.0f, 1.0f,
        3.0f, 2.0f, 0.0f, 1.0f
    };

    GLfloat birdColors[] = {
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &vaoBird);
    glBindVertexArray(vaoBird);

    // VBO pentru coordonate păsări
    glGenBuffers(1, &vboBird);
    glBindBuffer(GL_ARRAY_BUFFER, vboBird);
    glBufferData(GL_ARRAY_BUFFER, sizeof(birdVertices), birdVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // VBO pentru culori păsări
    glGenBuffers(1, &colorBird);
    glBindBuffer(GL_ARRAY_BUFFER, colorBird);
    glBufferData(GL_ARRAY_BUFFER, sizeof(birdColors), birdColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}


void CreateVAO() {
    vector<GLfloat> vertices;
    vector<GLfloat> colors;

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
    CreateVAO(); // fundal
    CreateShaders();
    CreateBirdVAO(); // pasari

    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
    myMatrix = resizeMatrix;
}

void InitializeStol() {
    float startX = 0.0f;
    float startY = 0.0f;

    stol.push_back({ startX, startY, 0.3f, glm::vec2(0.0f, 0.0f), 2.5f, 0.0f, 0.5f, 1.0f });

    stol.push_back({ startX - 15.0f, startY + 12.0f, 0.25f, glm::vec2(-15.0f, 12.0f), 2.3f, 0.2f, 0.5f, 1.0f });
    stol.push_back({ startX + 18.0f, startY - 8.0f, 0.27f, glm::vec2(18.0f, -8.0f), 2.3f, 0.1f, 0.5f, 1.0f });
    stol.push_back({ startX - 10.0f, startY - 14.0f, 0.2f, glm::vec2(-10.0f, -14.0f), 2.2f, 0.4f, 0.5f, 1.0f });
    stol.push_back({ startX + 22.0f, startY + 5.0f, 0.22f, glm::vec2(22.0f, 5.0f), 2.2f, 0.3f, 0.5f, 1.0f });

    stol.push_back({ startX - 5.0f, startY - 20.0f, 0.15f, glm::vec2(-5.0f, -20.0f), 2.1f, 0.6f, 0.5f, 1.0f });
    stol.push_back({ startX + 14.0f, startY - 18.0f, 0.17f, glm::vec2(14.0f, -18.0f), 2.1f, 0.5f, 0.5f, 1.0f });
    stol.push_back({ startX - 25.0f, startY - 10.0f, 0.18f, glm::vec2(-25.0f, -10.0f), 2.1f, 0.7f, 0.5f, 1.0f });
    stol.push_back({ startX - 28.0f, startY + 18.0f, 0.19f, glm::vec2(-28.0f, 18.0f), 2.1f, 0.8f, 0.5f, 1.0f });

    stol.push_back({ startX - 35.0f, startY + 25.0f, 0.09f, glm::vec2(-35.0f, 25.0f), 1.8f, 0.0f, 0.4f, 0.9f });
    stol.push_back({ startX + 10.0f, startY - 30.0f, 0.05f, glm::vec2(10.0f, -30.0f), 2.0f, 0.9f, 0.6f, 1.05f });
    stol.push_back({ startX - 20.0f, startY - 28.0f, 0.06f, glm::vec2(-20.0f, -28.0f), 1.9f, 1.0f, 0.55f, 1.1f });
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
    for (int i = cloudStartIndex; i < cloudEndIndex; i = i + numSegments + 2)
        glDrawArrays(GL_TRIANGLE_FAN, i, 42);

}

// pasare in forma de V
void DrawBird(glm::vec2 position, float scale = 1.0f, float wingAngle = 0.0f) {
    glBindVertexArray(vaoBird);

    myMatrix = glm::translate(resizeMatrix, glm::vec3(position, 0.0f));
    myMatrix = glm::scale(myMatrix, glm::vec3(scale, scale, 1.0f));

    // rotatie pe aripa pentru animatie
    glm::mat4 wingMatrix = myMatrix;

    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
    glUniform1f(colorFactorLocation, 1.0f);

    glLineWidth(3.5f);

    // se modifica doar coordonata y a varfurilor aripilor pentru simularea bataii pentru zbor
    GLfloat birdVertices[] = {
        // aripa stanga
        0.0f, 0.0f, 0.0f, 1.0f,
        -3.0f, 2.0f + wingAngle, 0.0f, 1.0f,
        // aripa dreapta
        0.0f, 0.0f, 0.0f, 1.0f,
        3.0f, 2.0f + wingAngle, 0.0f, 1.0f
    };

    // actualizam VBO cu noile coordonate pentru a vizualiza animatia
    glBindBuffer(GL_ARRAY_BUFFER, vboBird);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(birdVertices), birdVertices);

    glDrawArrays(GL_LINES, 0, 4);
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
    DrawTree(glm::vec2(-130, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-90, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-50, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(-10, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(30, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(70, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(110, -65), 0.3f, 0.8f);
    DrawTree(glm::vec2(130, -65), 0.3f, 0.8f);

    // randul de mijloc   
	for (int i = -160; i <= 160; i += 40)
        DrawTree(glm::vec2(i, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(-180, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(20, -65), 0.4f, 0.9f);
    DrawTree(glm::vec2(180, -65), 0.4f, 0.9f);

    // randul cel mai apropiat
	for (int i = -200; i <= 200; i += 50)
        DrawTree(glm::vec2(i, -65), 0.5f, 1.0f);

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

    
    // stolul de pasari
    for (const auto& bird : stol) {

        glm::vec2 finalPosition = glm::vec2(bird.currentX, bird.currentY);

        // animatia aripii
        float wingMovement = sin(wingTime * bird.wingSpeedFactor + bird.wingTimeOffset) * bird.wingAmplitude;

        DrawBird(finalPosition, bird.scale, wingMovement);
    }
    
	// pasarile care se spawneaza la apasarea tastei space
    for (auto& bird : spawnedBirds)
        if (bird.isOnScreen)
            DrawBird(glm::vec2(bird.xOffset, bird.yPosition), bird.scale, sin(wingTime + bird.wingPhase) * 0.5f);

    glFlush();
}

// functia de animatie a zborului
void UpdateAnimation(int value) 
{
    birdOffset += birdSpeed; // pozitia pasarilor se actualizeaza
    if (birdOffset > 500.0f) // reluam animatia atunci cand pasarile ies din ecran
        birdOffset = -300.0f;
    wingTime += 0.1f;   // actualizam timpul pentru urmatorul frame
    
	// efectul de interschimbare a pozitiilor pasarilor
    swapTimer += 0.016f; // ~16ms per frame (60 FPS)
    
    if (swapTimer >= swapInterval && !isSwapping) 
    {
		vector<int> visibleIndices; // ca sa nu iau pasari invizibile pe ecran sau foarte aproape de margini
        for (size_t i = 0; i < spawnedBirds.size(); ++i) 
                if (spawnedBirds[i].isOnScreen && spawnedBirds[i].xOffset > -100.0f && spawnedBirds[i].xOffset < 200.0f) 
                    visibleIndices.push_back(i);
     
    
        // daca am cel putin doua pasari pe ecran, ele pot sa faca schimb de pozitii
        if (visibleIndices.size() >= 2) // aleg oricare doua pasari 
         {

              int idx1 = visibleIndices[rand() % visibleIndices.size()];
              int idx2;
              do 
              {
                idx2 = visibleIndices[rand() % visibleIndices.size()];
              } while (idx1 == idx2);
            
              spawnedBirds[idx1].startX = spawnedBirds[idx1].xOffset;
              spawnedBirds[idx1].startY = spawnedBirds[idx1].yPosition;
              spawnedBirds[idx2].startX = spawnedBirds[idx2].xOffset;
              spawnedBirds[idx2].startY = spawnedBirds[idx2].yPosition;
          
              spawnedBirds[idx1].targetX = spawnedBirds[idx2].xOffset;
              spawnedBirds[idx1].targetY = spawnedBirds[idx2].yPosition;
  
              spawnedBirds[idx2].targetX = spawnedBirds[idx1].xOffset;
              spawnedBirds[idx2].targetY = spawnedBirds[idx1].yPosition;
   
              spawnedBirds[idx1].swapTimer = 0.0f;
              spawnedBirds[idx2].swapTimer = 0.0f;
   
              spawnedBirds[idx1].isSwapping = true;
              spawnedBirds[idx2].isSwapping = true;
     
              isSwapping = true;
        }
        
    swapTimer = 0.0f;
    }
    
    // daca avem pasari disponibile pentru interschimbare
    if (isSwapping) 
    {
         bool anyStillSwapping = false;
 
		 for (auto& bird : spawnedBirds) // realizez animatia independent pentru fiecare pasare (care este "marcata" pentru swap)
         {
               if (bird.isSwapping) 
               {
                   bird.swapTimer += 0.016f;
                
                   float progress = bird.swapTimer / swapDuration; 
        
				   if (progress >= 1.0f) // daca animatia s-a finalizat, setam pozitia finala
                   {
                        bird.xOffset = bird.targetX;
                        bird.yPosition = bird.targetY;
                        bird.isSwapping = false;
                   } 
                   else 
                   {
                      // ne aflam mid animatie
                      float smoothProgress = progress * progress * (3.0f - 2.0f * progress); // smoothstep
                      bird.xOffset = bird.startX + (bird.targetX - bird.startX) * smoothProgress;
                      bird.yPosition = bird.startY + (bird.targetY - bird.startY) * smoothProgress;
                      anyStillSwapping = true;
                    }
                }
         }

        if (!anyStillSwapping) 
            isSwapping = false;   
    }

    // actualizam pozitia pasarilor spawnate (doar pentru cele care NU sunt in swap)
    for (auto& bird : spawnedBirds)
    {
        if (bird.isOnScreen && !bird.isSwapping)
        {
            bird.xOffset += bird.speed;
            if (bird.xOffset > 300.0f) 
                bird.isOnScreen = false;
        }
    }
    
	// eliberam memoria de pasari care au iesit din ecran
    spawnedBirds.erase(std::remove_if(spawnedBirds.begin(), spawnedBirds.end(), [](const SpawnedBird& b) { return !b.isOnScreen; }), spawnedBirds.end());
    
    glutPostRedisplay(); // refacem scena
    glutTimerFunc(16, UpdateAnimation, 0);
}

void MouseMotion(int mouseX, int mouseY)
{
    float screenWidth = (float)glutGet(GLUT_WINDOW_WIDTH);
    float screenHeight = (float)glutGet(GLUT_WINDOW_HEIGHT);

    float vao_width = vao_xMax_current - vao_xMin_current;
    float vao_height = vao_yMax_current - vao_yMin_current;

	// pozitia mouse ului mapata in coordonate Vao curent
    float baseTargetX = vao_xMin_current + vao_width * ((float)mouseX / screenWidth);
    float baseTargetY = vao_yMin_current + vao_height * ((screenHeight - (float)mouseY) / screenHeight);

    for (auto& bird : stol) { 

        // pozitia in functie de acel offset pentru a nu se suprapune pasarile
        float individualTargetX = baseTargetX + bird.permanentOffset.x;
        float individualTargetY = baseTargetY + bird.permanentOffset.y;

		// se aplica interpolarea liniara pentru a face miscarea mai fluida
        bird.currentX = (1.0f - bird.lerpFactor) * bird.currentX + bird.lerpFactor * individualTargetX;
        bird.currentY = (1.0f - bird.lerpFactor) * bird.currentY + bird.lerpFactor * individualTargetY;
        
        bird.currentX += sin(wingTime + bird.wingTimeOffset) * 0.5f;
    }

    glutPostRedisplay();
}

void MouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
		// cand se apasa butonul stang al mouse ului
        if (state == GLUT_DOWN) {
            isMouseTracking = true;
            MouseMotion(x, y);
        }
        else if (state == GLUT_UP) {
            isMouseTracking = false;
        }
    }
}

void IdleFunction() {
    if (!isMouseTracking) {
        for (auto& bird : stol) {
            bird.currentX += autoFlySpeed;

			// apare stolul in stanga iar cand iese in dreapta
             if (bird.currentX > vao_xMax_current + 50.0f) {
                 bird.currentX = vao_xMin_current - 50.0f;
             }
        }
    }

    glutPostRedisplay();
}
void AddNewBird() 
{
    SpawnedBird newBird;
    newBird.xOffset = -200.0f;
    newBird.yPosition = 10.0f + static_cast<float>(rand() % 60); // coordonata y random in zona de cer, evitand astfel zona copacilor
    newBird.targetX = newBird.xOffset; // inițial, targetX = xOffset
    newBird.targetY = newBird.yPosition; // inițial, targetY = yPosition
    newBird.startX = newBird.xOffset;
    newBird.startY = newBird.yPosition;
    newBird.swapTimer = 0.0f;
    newBird.scale = 1.5f + static_cast<float>(rand() % 100) / 100.0f; // dimensiune random
	newBird.speed = 0.4f + static_cast<float>(rand() % 60) / 100.0f; // viteza random
    newBird.wingPhase = static_cast<float>(rand() % 100) / 100.0f * 6.28f;
    newBird.isOnScreen = true;
    newBird.isSwapping = false;
    spawnedBirds.push_back(newBird);
}


void KeyboardFunc(unsigned char key, int x, int y) {
    if (key == ' ' || key == 32)  // tasta space
        AddNewBird();
}

void Cleanup() {
    glDeleteProgram(ProgramId);
    glDeleteBuffers(1, &vboBg);
    glDeleteBuffers(1, &colorBg);
    glDeleteVertexArrays(1, &vaoBg);
    glDeleteBuffers(1, &vboBird);
    glDeleteBuffers(1, &colorBird);
    glDeleteVertexArrays(1, &vaoBird);
}

void ReshapeFunction(int width, int height)
{
    glViewport(0, 0, width, height);

    if (height == 0) height = 1;

    // raportul de aspect al ferestrei curente
    float aspect = (float)width / (float)height;

    float viewHeight = yMax - yMin;
    // viewWidth este ajustata in functie de aspect
    float viewWidth = viewHeight * aspect;

    float centerX = (xMax + xMin) / 2.0f;
    float centerY = (yMax + yMin) / 2.0f;

    float halfWidth = viewWidth / 2.0f;
    float halfHeight = viewHeight / 2.0f;

    resizeMatrix = glm::ortho(
        centerX - halfWidth,   // Left
        centerX + halfWidth,   // Right
        centerY - halfHeight,  // Bottom
        centerY + halfHeight   // Top
    );

    // folosite in MouseMotion pentru maparea pixel-unitate
    vao_xMin_current = centerX - halfWidth;
    vao_xMax_current = centerX + halfWidth;
    vao_yMin_current = centerY - halfHeight;
    vao_yMax_current = centerY + halfHeight;
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Proiect 1");

    glewInit();
    
	// initializam generatorul de numere random
    srand(static_cast<unsigned int>(time(nullptr)));

    Initialize();
	InitializeStol();

    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion); 
    glutIdleFunc(IdleFunction);

    glutDisplayFunc(RenderFunction);
    glutReshapeFunc(ReshapeFunction);
    glutKeyboardFunc(KeyboardFunc); // handler pentru keyboard input
    glutCloseFunc(Cleanup);

    glutTimerFunc(0, UpdateAnimation, 0);

    glutMainLoop();
}