
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera/Camera.h"
#include "../external/Angel/src/InitShader.cpp"
#include "global/GlobalConfig.h"
#include "utils/Axes.h"
#include "spider/Spider.h"
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
    // Set the background color to purple
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);






    // 1) Load and use shaders for the abdomen
    GLuint abdomenProgram = InitShader("abdomen_vertex.glsl", "abdomen_fragment.glsl");
    GLuint abdomenMVLoc   = glGetUniformLocation(abdomenProgram, "model_view");
    GLuint abdomenPLoc    = glGetUniformLocation(abdomenProgram, "projection");

    spider::Spider spider(abdomenProgram);

    // Print initial leg tip ground contacts
    std::cout << "Initial Leg Tip Ground Contacts (World Y=0):" << std::endl;
    const std::vector<vec3>& contacts = spider.getInitialLegTipGroundContacts();
    for (size_t i = 0; i < contacts.size(); ++i) {
        std::cout << "Leg " << i << ": ("
                  << std::fixed << std::setprecision(3) << contacts[i].x << ", "
                  << std::fixed << std::setprecision(3) << contacts[i].y << ", "
                  << std::fixed << std::setprecision(3) << contacts[i].z << ")" << std::endl;
    }
    std::cout << "---------------------------------------------" << std::endl;

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
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(GLFW_KEY_SPACE);

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
        // Add other spider controls here if needed (e.g., turning)

        spider.update(deltaTime);

        // --- TEST BLOCK for IK on Leg 7 ---
        // --- TEST BLOCK for IK on Leg 7 ---
        static bool leg7_ik_applied_once = false; // Renamed for clarity
        if (spider.getPosition().z > 2.0f && !leg7_ik_applied_once ) {
            const std::vector<vec3>& initial_contacts = spider.getInitialLegTipGroundContacts();
            if (initial_contacts.size() > 7) {
                vec3 leg7_initial_contact = initial_contacts[7];
                vec3 new_target_for_leg7 = vec3(leg7_initial_contact.x, -1.0f, leg7_initial_contact.z);

                std::cout << "Spider Z > 2.0. Applying IK for Leg 7 to target Y=1.0 ONCE." << std::endl;
                // ... (print target) ...

                spider.runIKForLeg(7, new_target_for_leg7);
                leg7_ik_applied_once = true; // Ensures IK runs only once for this condition
            }
        }
        // --- END OF TEST BLOCK ---
        // --- END OF TEST BLOCK ---

        mat4 Projection = Perspective( 45.0f, 4.0f/3.0f, 0.1f, 100.0f );
        mat4 View       = camera.getViewMatrix();
        mat4 axesMV = View;

        glUniformMatrix4fv(projLoc, 1, GL_TRUE, Projection);
        glUniformMatrix4fv(mvLoc,   1, GL_TRUE, axesMV);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        axes.draw(axesMVLoc, axesPLoc, axesMV, Projection);

        spider.draw(abdomenMVLoc, abdomenPLoc, View, Projection);


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
