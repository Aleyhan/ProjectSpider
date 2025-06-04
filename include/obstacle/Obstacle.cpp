#include "Obstacle.h"

Obstacle::Obstacle(const vec3& pos, float size, GLuint shaderProgram)
    : position_(pos), size_(size), shaderProgram_(shaderProgram) {}

void Obstacle::draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& view, const mat4& projection) {
    mat4 model = Translate(position_) * Scale(size_);
    mat4 mv = view * model;

    glUseProgram(shaderProgram_);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, mv);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projection);

    glutSolidCube(1.0f); // Angel GLUT fonksiyonu
}

const vec3& Obstacle::getPosition() const {
    return position_;
}

float Obstacle::getSize() const {
    return size_;
}