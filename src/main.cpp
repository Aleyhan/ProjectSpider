
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

#include "spider/LegSegment.h"

using namespace Angel;


// projection + model_view matrices
GLuint programID, projLoc, mvLoc;
// Kamera hedefi (spider dünya merkezinde), uzaklık 5 birim

float spiderX = 0.0f;
float speed = 0.01f;

Camera camera;


int main() {
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
            int c = (((i & 8) == 0) ^ ((j & 8) == 0)) * 255;
            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
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
    GLuint abdomenProgram = InitShader("abdomen_vertex.glsl", "abdomen_fragment.glsl");
    GLuint abdomenMVLoc   = glGetUniformLocation(abdomenProgram, "model_view");
    GLuint abdomenPLoc    = glGetUniformLocation(abdomenProgram, "projection");

    spider::Spider spider(abdomenProgram);


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
        // keyboard
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_W);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_S);
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_A);
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_D);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_C);
        // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_SPACE);

        // Trigger spider jump on spacebar
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            spider.jump(deltaTime, 1.0f); // 1.0f is the jump duration
        }

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


        // Add other spider controls here if needed (e.g., turning)

        spider.update(deltaTime);
        // Prevent spider from going below ground
        vec3 pos = spider.getPosition();
        if (pos.y < 0.0f) {
            pos.y = 0.0f;
            spider.setPosition(pos);  // Bu fonksiyon mevcutsa
        }


        mat4 Projection = Perspective( 45.0f, 4.0f/3.0f, 0.1f, 100.0f );
        mat4 View       = camera.getViewMatrix();
        mat4 axesMV = View;

        glUniformMatrix4fv(projLoc, 1, GL_TRUE, Projection);
        glUniformMatrix4fv(mvLoc,   1, GL_TRUE, axesMV);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        axes.draw(axesMVLoc, axesPLoc, axesMV, Projection);

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
