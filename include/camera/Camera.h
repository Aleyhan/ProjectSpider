#pragma once
#include "../external/Angel/inlcude/Angel/Angel.h"
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera();

    void processKeyboard(int key);
    mat4 getViewMatrix() const;

private:
    vec3 _position;
    vec3 _front;
    vec3 _up;
    vec3 _right;
    vec3 _worldUp;

    float _yaw;   // in degrees
    float _pitch; // in degrees
    float _speed;
    float _sensitivity;

    void updateVectors();
};
