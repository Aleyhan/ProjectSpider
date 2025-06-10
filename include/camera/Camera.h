#pragma once
#include "../external/Angel/inlcude/Angel/Angel.h"
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera();

    void processKeyboard(int key);
    mat4 getViewMatrix() const;

    void setPosition(const vec3& pos);
    void lookAt(const vec3& target);

    void setVerticalOffset(float offset);
    

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
    float _verticalOffset;

    void updateVectors();
    void updateViewMatrix();

    vec3 _target;
};
