#include "obstacle/Obstacle.h"
#include "model/Model.h"
#include <vector>

Obstacle::Obstacle(vec3 position, float size, int pointValue, GLuint shaderProgram, const std::string& modelPath)
    : position(position), size(size), pointValue(pointValue), shader(shaderProgram), model(modelPath) {}

void Obstacle::draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& viewMatrix, const mat4& projMatrix) {
    mat4 modelMat = Translate(position) * Scale(size);
    mat4 modelView = viewMatrix * modelMat;

    glUseProgram(shader);

    GLint colorLoc = glGetUniformLocation(shader, "uColor");
    if (pointValue < 0)
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f); // ceza
    else
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // ödül

    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelView);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projMatrix);

    model.draw(viewMatrix, projMatrix);
}

void Obstacle::setModel(const std::string& modelPath) {
    model = Model(modelPath);
}

const vec3& Obstacle::getPosition() const {
    return position;
}

int Obstacle::getPointValue() const {
    return pointValue;
}