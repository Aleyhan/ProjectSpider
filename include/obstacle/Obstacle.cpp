#include "obstacle/Obstacle.h"
#include <vector>

Obstacle::Obstacle(vec3 position, float size, int pointValue, GLuint shaderProgram)
    : position(position), size(size), pointValue(pointValue), shader(shaderProgram) {
    std::vector<GLfloat> vertices = {
        -0.5, -0.5, -0.5,  +0.5, -0.5, -0.5,  +0.5, +0.5, -0.5,  -0.5, +0.5, -0.5,
        -0.5, -0.5, +0.5,  +0.5, -0.5, +0.5,  +0.5, +0.5, +0.5,  -0.5, +0.5, +0.5
    };
    GLuint indices[] = {
        0,1,2, 2,3,0,  4,5,6, 6,7,4,
        0,1,5, 5,4,0,  2,3,7, 7,6,2,
        0,3,7, 7,4,0,  1,2,6, 6,5,1
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glBindVertexArray(0);
}

void Obstacle::draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& viewMatrix, const mat4& projMatrix) {
    mat4 model = Translate(position) * Scale(size);
    mat4 modelView = viewMatrix * model;

    glUseProgram(shader);

    GLint colorLoc = glGetUniformLocation(shader, "uColor");
    if (pointValue < 0)
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f); // siyah (ceza)
    else
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // beyaz (ödül)

    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelView);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projMatrix);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

const vec3& Obstacle::getPosition() const {
    return position;
}

int Obstacle::getPointValue() const {
    return pointValue;
}