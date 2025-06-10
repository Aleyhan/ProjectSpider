// Camera.cpp
#include "camera/Camera.h"
#include <cmath>

constexpr float PI = 3.14159265359f;

inline float radians(float degrees) {
    return degrees * (PI / 180.0f);
}

template<typename T>
inline T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

Camera::Camera()
    : _position(0.0f, 1.0f, 5.0f),
      _worldUp(0.0f, 1.0f, 0.0f),
      _yaw(-90.0f),  // looking down -Z
      _pitch(0.0f),
      _speed(0.1f),
      _sensitivity(2.0f)
{
    updateVectors();
}

void Camera::processKeyboard(int key) {
    if (key == GLFW_KEY_W) _position += _speed * _front;
    if (key == GLFW_KEY_S) _position -= _speed * _front;
    if (key == GLFW_KEY_A) _position -= _speed * _right;
    if (key == GLFW_KEY_D) _position += _speed * _right;
    if (key == GLFW_KEY_C)       _position += _speed * _worldUp;
    if (key == GLFW_KEY_SPACE)  _position -= _speed * _worldUp;

    if (key == GLFW_KEY_LEFT)   _yaw   -= _sensitivity;
    if (key == GLFW_KEY_RIGHT)  _yaw   += _sensitivity;
    if (key == GLFW_KEY_UP)     _pitch += _sensitivity;
    if (key == GLFW_KEY_DOWN)   _pitch -= _sensitivity;

    // Clamp pitch
    _pitch = clamp(_pitch, -89.0f, 89.0f);

    updateVectors();
}

mat4 Camera::getViewMatrix() const {
    return LookAt(_position, _position + _front, _up);
}

void Camera::updateVectors() {
    float radYaw = radians(_yaw);
    float radPitch = radians(_pitch);

    vec3 front;
    front.x = cos(radPitch) * cos(radYaw);
    front.y = sin(radPitch);
    front.z = cos(radPitch) * sin(radYaw);
    _front = normalize(front);

    _right = normalize(cross(_front, _worldUp));
    _up    = normalize(cross(_right, _front));
}

void Camera::setPosition(const vec3& position) {
    _position = position;
}

void Camera::lookAt(const vec3& target) {
    _front = normalize((target + vec3(0.0f, -2.0f, 0.0f)) - _position);
    _right = normalize(cross(_front, _worldUp));
    _up = normalize(cross(_right, _front));
}
