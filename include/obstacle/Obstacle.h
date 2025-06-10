#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include "model/Model.h"

class Obstacle {
public:
    Obstacle(vec3 position, float size, int pointValue, GLuint shaderProgram, const std::string& modelPath);
    void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& viewMatrix, const mat4& projMatrix);
    const vec3& getPosition() const;
    int getPointValue() const;
    void setModel(const std::string& modelPath);

private:
    vec3 position;
    float size;
    int pointValue;
    GLuint vao, vbo;
    GLuint shader;
    Model model;
};

#endif // OBSTACLE_H