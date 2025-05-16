#include "spider/Eye.h"
#include <vector>
#include <cmath>
#include "global/GlobalConfig.h"

namespace spider {

Eye::Eye(GLuint shaderProgram)
    : _program(shaderProgram) {
    initMesh();
}

Eye::~Eye() {
    cleanup();
}

void Eye::cleanup() {
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
    }
}

void Eye::initMesh() {
    const int stacks = 10, slices = 10;
    const float r = ABDOMEN_RADIUS * HEAD_SCALE*0.25f;

    std::vector<GLfloat> interleaved;
    std::vector<GLuint> indices;

    for (int i = 0; i <= stacks; ++i) {
        float v = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float u = 2.0f * M_PI * j / slices;

            float x = r * std::sin(v) * std::cos(u);
            float y = r * std::sin(v) * std::sin(u);
            float z = r * std::cos(v);

            vec3 normal = normalize(vec3(x, y, z));

            interleaved.insert(interleaved.end(), {x, y, z, normal.x, normal.y, normal.z});
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int row1 = i * (slices + 1) + j;
            int row2 = row1 + slices + 1;

            indices.push_back(static_cast<GLuint>(row1));
            indices.push_back(static_cast<GLuint>(row2));
            indices.push_back(static_cast<GLuint>(row1 + 1));

            indices.push_back(static_cast<GLuint>(row2));
            indices.push_back(static_cast<GLuint>(row2 + 1));
            indices.push_back(static_cast<GLuint>(row1 + 1));

        }
    }

    _indexCount = GLsizei(indices.size());

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(GLfloat), interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

void Eye::draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& modelMatrix, const mat4& projMatrix) const {
    glUseProgram(_program);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projMatrix);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

}
