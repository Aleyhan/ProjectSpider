// #define TEST_LEG // comment this to switch back to real spider

#ifdef TEST_LEG

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera/Camera.h"
#include "../external/Angel/src/InitShader.cpp"
#include "global/GlobalConfig.h"
#include "utils/Axes.h"
#include "spider/Leg.h"
#include <cmath>

Camera camera;

int main() {
    if (!glfwInit()) return -1;

    // macOS compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Leg Test", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    // Load shader
    GLuint shaderProgram = InitShader("abdomen_vertex.glsl", "abdomen_fragment.glsl");
    GLuint modelViewLoc = glGetUniformLocation(shaderProgram, "model_view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Create a Leg object
    spider::Leg leg(shaderProgram);

    mat4 P = Perspective(45.0f, 1.0f, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window)) {
        // Process camera input
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_W);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_S);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_A);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_D);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_C);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_SPACE);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_LEFT);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_RIGHT);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.processKeyboard(GLFW_KEY_UP);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_DOWN);
        // Add somewhere in your key handling section
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) leg.setJointAngle(0, leg.getJointAngle(0) + 1.0f);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) leg.setJointAngle(0, leg.getJointAngle(0) - 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        mat4 V = camera.getViewMatrix();
        mat4 base = V * Translate(0.0f, 0.0f, 0.0f);
        static float time = 0.0f;
        time += 0.01f;

        // Animate multiple joints with different phases
        leg.setJointAngle(0, 30.0f + 15.0f * sin(time));
        leg.setJointAngle(1, -20.0f + 10.0f * sin(time + 0.5f));
        leg.setJointAngle(2, 40.0f + 20.0f * sin(time + 1.0f));
        leg.setJointAngle(3, -30.0f + 15.0f * sin(time + 1.5f));

        leg.draw(modelViewLoc, projectionLoc, base, P);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

#else

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera/Camera.h"
#include "../external/Angel/src/InitShader.cpp"
#include "global/GlobalConfig.h"
#include "utils/Axes.h"
#include "spider/Spider.h"
#include "spider/Abdomen.h"

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


    // 4) Main render loop
    while(!glfwWindowShouldClose(window)) {
        // keyboard
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_W);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_S);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_A);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_D);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_C);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_SPACE);

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_LEFT);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_RIGHT);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.processKeyboard(GLFW_KEY_UP);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.processKeyboard(GLFW_KEY_DOWN);

        // Matrisleri al:
        mat4 Projection = Perspective( 45.0f, 4.0f/3.0f, 0.1f, 100.0f );
        mat4 View       = camera.getViewMatrix();
        mat4 MV         = View * Translate(1.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(projLoc, 1, GL_TRUE, Projection);
        glUniformMatrix4fv(mvLoc,   1, GL_TRUE, MV);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        axes.draw(axesMVLoc, axesPLoc, MV, Projection);

        spider.draw(abdomenMVLoc, abdomenPLoc, MV, Projection);


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
#endif // TEST_LEG