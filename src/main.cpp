#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera/Camera.h"
#include "../external/Angel/src/InitShader.cpp"
#include "global/GlobalConfig.h"
#include "utils/Axes.h"
#include "spider/Spider.h"
#include "spider/Leg.h"
#include "spider/Abdomen.h"
#include <vector>
#include <iomanip>
#include "../external/stb_easy_font.h"

#include "spider/LegSegment.h"
#include "obstacle/Obstacle.h"

using namespace Angel;


// projection + model_view matrices
GLuint programID, projLoc, mvLoc;


float spiderX = 0.0f;
int score = 0;
float speed = 0.01f;

Camera camera;

std::vector<Obstacle> obstacles;
std::vector<spider::Spider> aiSpiders;




void setupObstacles(GLuint shaderProgram);

void initAISpiders(GLuint cephalothoraxShader, GLuint abdomenShader, GLuint legShader, GLuint eyeShader) {
    for (int i = 0; i < 5; ++i) {
        float x = (rand() % 400 - 200) / 10.0f;
        float z = (rand() % 400 - 200) / 10.0f;
        spider::Spider aiSpider(cephalothoraxShader, abdomenShader, legShader, eyeShader);
        aiSpider.setPosition(vec3(x, 0.7f, z));
        aiSpiders.push_back(aiSpider);
    }
}



int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    //***********************************************************************************
    //***********************************************************************************
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }



    // macOS compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Spider Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);


    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    //***********************************************************************************
    //***********************************************************************************
    // Checkerboard ground setup
    // Create checkerboard texture
    const int checkImageWidth = 64;
    const int checkImageHeight = 64;
    GLubyte checkImage[checkImageHeight][checkImageWidth][3];

    for (int i = 0; i < checkImageHeight; i++) {
        for (int j = 0; j < checkImageWidth; j++) {
            // Rastgele yeşil tonları oluştur
            GLubyte baseGreen = 100 + (rand() % 50);  // 100-150 arası yeşil tonu
            checkImage[i][j][0] = (GLubyte)(baseGreen * 0.4);  // R değeri (yeşilin %40'ı)
            checkImage[i][j][1] = (GLubyte)baseGreen;          // G değeri (ana yeşil)
            checkImage[i][j][2] = (GLubyte)(baseGreen * 0.4);  // B değeri (yeşilin %40'ı)
        }
    }


    GLuint checkerTexture;
    glGenTextures(1, &checkerTexture);
    glBindTexture(GL_TEXTURE_2D, checkerTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, checkImageWidth, checkImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    float groundVertices[] = {
        // positions           // normals       // tex coords
        -20.0f, 0.0f, -20.0f,   0, 1, 0,         0.0f, 0.0f,
        20.0f, 0.0f, -20.0f,   0, 1, 0,         8.0f, 0.0f,
        20.0f, 0.0f,  20.0f,   0, 1, 0,         8.0f, 8.0f,
        -20.0f, 0.0f,  20.0f,   0, 1, 0,         0.0f, 8.0f
    };

    GLuint groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    GLuint groundProgram = InitShader("shaders/ground_vertex.glsl", "shaders/ground_fragment.glsl");
    GLuint groundMVLoc = glGetUniformLocation(groundProgram, "model_view");
    GLuint groundPLoc  = glGetUniformLocation(groundProgram, "projection");

    // Set the background color to purple
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);






    // 1) Load and use shaders for the abdomen
    GLuint cephalothoraxShader = InitShader("../shaders/spider_vertex.glsl", "../shaders/cephalothorax_fragment.glsl");
    GLuint abdomenShader = InitShader("../shaders/spider_vertex.glsl", "../shaders/abdomen_fragment.glsl");
    GLuint legShader = InitShader("../shaders/spider_vertex.glsl", "../shaders/leg_fragment.glsl");
    GLuint eyeShader = InitShader("../shaders/spider_vertex.glsl", "../shaders/eye_fragment.glsl");
    GLuint obstacleShader = InitShader("../shaders/obstacle_vertex.glsl", "../shaders/obstacle_fragment.glsl");
    GLuint obstacleMVLoc = glGetUniformLocation(obstacleShader, "model_view");
    GLuint obstaclePLoc = glGetUniformLocation(obstacleShader, "projection");
    GLuint abdomenMVLoc   = glGetUniformLocation(abdomenShader, "model_view");
    GLuint abdomenPLoc    = glGetUniformLocation(abdomenShader, "projection");

    spider::Spider spider(cephalothoraxShader, abdomenShader, legShader, eyeShader);
    initAISpiders(cephalothoraxShader, abdomenShader, legShader, eyeShader);
    camera.setPosition(spider.getPosition() + vec3(0.0f, 5.0f, 10.0f));
    camera.lookAt(spider.getPosition());
    setupObstacles(obstacleShader);




    // 2) setting the axes shader
    GLuint axesProgram = InitShader("../shaders/axes_vertex.glsl", "../shaders/axes_fragment.glsl");
    GLuint axesMVLoc   = glGetUniformLocation(axesProgram, "model_view");
    GLuint axesPLoc    = glGetUniformLocation(axesProgram, "projection");

    Axes axes(axesProgram);

    float lastFrameTime = 0.0f;

    // 4) Main render loop
    while(!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        for (size_t i = 0; i < aiSpiders.size(); ++i) {
            auto& ai = aiSpiders[i];
            float t = static_cast<float>(glfwGetTime()) + i * 1.8f;
            ai.startWalkingForward();
            if (fmod(t, 4.0f) < 2.0f) {
                ai.startTurningLeft();
                ai.stopTurningRight();
            } else {
                ai.startTurningRight();
                ai.stopTurningLeft();
            }
            ai.update(deltaTime);
        }

        // keyboard
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_W);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_S);
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_A);
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_D);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_C);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_SPACE);

        /*
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            spider.jump(deltaTime, 1.0f); // 1.0f is the jump duration
        }
        */

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_LEFT);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_RIGHT);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.processKeyboard(GLFW_KEY_UP);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_DOWN);

        // Spider movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            spider.startWalkingForward();
        } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            spider.startWalkingBackward();
        } else {
            spider.stopWalkingForward();
            spider.stopWalkingBackward();
        }

        // Add turning logic here
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            spider.startTurningLeft();
        } else {
            spider.stopTurningLeft();
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            spider.startTurningRight();
        } else {
            spider.stopTurningRight();
        }

        // Spider movement
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            spider.moveBodyUp();
        } else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            spider.moveBodyDown();
        }

       if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS ) {
            spider.jumpTriggered = true; // Define and initialize the global variable
       }

        for (size_t i = 0; i < aiSpiders.size(); ++i) {
            auto& ai = aiSpiders[i];
            float t = static_cast<float>(glfwGetTime()) + i * 1.8f;

            ai.startWalkingForward();

            if (fmod(t, 4.0f) < 2.0f) {
                ai.startTurningLeft();
                ai.stopTurningRight();
            } else {
                ai.startTurningRight();
                ai.stopTurningLeft();
            }

            ai.update(deltaTime);
        }


        // Add other spider controls here if needed (e.g., turning)

        spider.update(deltaTime);

        vec3 spiderPos = spider.getPosition();
        camera.setPosition(spiderPos + vec3(0.0f, 5.0f, 15.0f));  // Yüksekliği ve uzaklığı ayarla
        camera.lookAt(spiderPos);

        vec3 spiderPosCollision = spider.getPosition(); // Assumes getPosition() returns current position of spider
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            vec3 diff = spiderPosCollision - it->getPosition();
            float dist = sqrt(dot(diff, diff));
            if (dist < 1.0f) { // Adjust collision threshold if needed
                score += it->getPointValue();
                std::cout << "Collision! Score: " << score << std::endl;

                // Display score as overlay using OpenGL
                std::string scoreText = "Score: " + std::to_string(score);
                glfwSetWindowTitle(window, scoreText.c_str());

                it = obstacles.erase(it); // Remove obstacle after collision
            } else {
                ++it;
            }
        }


        mat4 Projection = Perspective( 45.0f, 4.0f/3.0f, 0.1f, 100.0f );
        mat4 View       = camera.getViewMatrix();
        mat4 axesMV = View;

        glUniformMatrix4fv(projLoc, 1, GL_TRUE, Projection);
        glUniformMatrix4fv(mvLoc,   1, GL_TRUE, axesMV);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        axes.draw(axesMVLoc, axesPLoc, axesMV, Projection);

        for (auto& obs : obstacles) {
            obs.draw(obstacleMVLoc, obstaclePLoc, View, Projection);
        }

        for (auto& ai : aiSpiders) {
            ai.draw(abdomenMVLoc, abdomenPLoc, View, Projection);
        }

        spider.draw(abdomenMVLoc, abdomenPLoc, View, Projection);


        // Draw ground
        glUseProgram(groundProgram);
        glUniformMatrix4fv(groundMVLoc, 1, GL_TRUE, View);
        glUniformMatrix4fv(groundPLoc,  1, GL_TRUE, Projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, checkerTexture);
        glUniform1i(glGetUniformLocation(groundProgram, "checkerTex"), 0);

        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);



        // Position health bar to top-right using OpenGL overlay (drawn with immediate mode)
        int health = std::max(0, 10 - score); // Health from 10 to 0
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1); // Top-left origin

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);

        for (int i = 0; i < 10; ++i) {
            if (i >= health) glColor3f(0.3f, 0.3f, 0.3f); // gray (empty)
            else glColor3f(0.0f, 1.0f, 0.0f); // green (full)

            float barWidth = 12;
            float barHeight = 20;
            float spacing = 3;
            float x = 800 - (barWidth + spacing) * (10 - i) - 10;
            float y = 560; // push closer to top edge

            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + barWidth, y);
            glVertex2f(x + barWidth, y + barHeight);
            glVertex2f(x, y + barHeight);
            glEnd();
        }

        glEnable(GL_DEPTH_TEST);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        /*
        // Score box (top-right corner)
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
        char textBuffer[99999];

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1); // top-left origin

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        // Draw white background box
        glColor3f(1.0f, 1.0f, 1.0f);
        float boxWidth = 110;
        float boxHeight = 24;
        float boxX = 800 - boxWidth - 20; // Increase margin from right edge
        float boxY = 10; // Top margin

        glBegin(GL_QUADS);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight);
        glVertex2f(boxX, boxY + boxHeight);
        glEnd();

        // Draw red border
        glLineWidth(2.0f);  // Ensure the line is thick enough to see
        glColor3f(1.0f, 0.0f, 0.0f); // red
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight);
        glVertex2f(boxX, boxY + boxHeight);
        glEnd();

        glPushMatrix();
        glTranslatef(boxX + 8, boxY + 6, 0); // Slight padding inside box
        glColor3f(0.0f, 0.0f, 0.0f);
        glEnableClientState(GL_VERTEX_ARRAY);
        int quads = stb_easy_font_print(0, 0, scoreText, nullptr, textBuffer, sizeof(textBuffer));
        glVertexPointer(2, GL_FLOAT, 16, textBuffer);
        glDrawArrays(GL_QUADS, 0, quads * 4);
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }






    //***********************************************************************************
    //***********************************************************************************
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;


}

void setupObstacles(GLuint shaderProgram) {
    for (int i = 0; i < 10; ++i) {
        float x = static_cast<float>((rand() % 400 - 200) / 10.0f); // -20.0f to +20.0f
        float z = static_cast<float>((rand() % 400 - 200) / 10.0f);
        int type = rand() % 4;
        int pointValue;
        std::string modelPath;
        switch (type) {
            case 0: // Kale
                pointValue = 5;
                modelPath = "models/rook.stl";
                break;
            case 1: // Vezir
                pointValue = 3;
                modelPath = "models/queen.stl";
                break;
            case 2: // Piyon
                pointValue = 1;
                modelPath = "models/pawn.stl";
                break;
            case 3: // Engelleyici taş
            default:
                pointValue = -2;
                modelPath = "models/blocker.stl";
                break;
        }
        obstacles.push_back(Obstacle(vec3(x, 0.5f, z), 1.0f, pointValue, shaderProgram, modelPath));
    }
}
