// Axes.cpp
#include "utils/Axes.h"
#include <vector>

// Unit‐cube corners (centered at origin, size 1)
static constexpr GLfloat cubeVerts[] = {
    // x,    y,    z
    -0.5f, -0.5f, -0.5f,  // 0
     0.5f, -0.5f, -0.5f,  // 1
     0.5f,  0.5f, -0.5f,  // 2
    -0.5f,  0.5f, -0.5f,  // 3
    -0.5f, -0.5f,  0.5f,  // 4
     0.5f, -0.5f,  0.5f,  // 5
     0.5f,  0.5f,  0.5f,  // 6
    -0.5f,  0.5f,  0.5f   // 7
};

// 12 triangles = 36 indices
static constexpr GLuint cubeIdx[] = {
    // back face
    0,1,2,   2,3,0,
    // front face
    4,5,6,   6,7,4,
    // left face
    0,3,7,   7,4,0,
    // right face
    1,5,6,   6,2,1,
    // bottom face
    0,1,5,   5,4,0,
    // top face
    3,2,6,   6,7,3
};

Axes::Axes(GLuint program) : _program(program) {
    const float axisLength = 5.0f; // Make the axes longer
    const float axisThick  = 0.025f; // Adjust thickness if needed

    // Colors for X, Y, Z axes
    const GLfloat colors[3][3] = {
        {1.0f, 0.0f, 0.0f},  // X = red
        {0.0f, 1.0f, 0.0f},  // Y = green
        {0.0f, 0.0f, 1.0f}   // Z = blue
    };

    std::vector<GLfloat> interleaved;
    std::vector<GLuint>  indices;

    // For each axis, bake a scaled/transformed cube into one big mesh
    for (int axis = 0; axis < 3; ++axis) {
        // Determine scale and translation for this axis
        float sx = (axis == 0 ? axisLength : axisThick);
        float sy = (axis == 1 ? axisLength : axisThick);
        float sz = (axis == 2 ? axisLength : axisThick);

        // The translation will now effectively start the axes at the origin
        float tx = 0.0f;
        float ty = 0.0f;
        float tz = 0.0f;

        // Append transformed cube verts + color
        GLuint baseIndex = GLuint(interleaved.size() / 6);
        for (int i = 0; i < 8; ++i) {
            // original cube vertex
            float x0 = cubeVerts[3*i + 0];
            float y0 = cubeVerts[3*i + 1];
            float z0 = cubeVerts[3*i + 2];

            // scale + translate
            float x = x0 * sx + tx;
            float y = y0 * sy + ty;
            float z = z0 * sz + tz;

            // push pos
            interleaved.push_back(x);
            interleaved.push_back(y);
            interleaved.push_back(z);
            // push color
            interleaved.push_back(colors[axis][0]);
            interleaved.push_back(colors[axis][1]);
            interleaved.push_back(colors[axis][2]);
        }

        // Append indices (offset by baseIndex)
        for (GLuint idx : cubeIdx) {
            indices.push_back(baseIndex + idx);
        }
    }

    _indexCount = GLsizei(indices.size());

    // Upload to GPU
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        interleaved.size() * sizeof(GLfloat),
        interleaved.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW
    );

    // position = location 0, color = location 1
    constexpr GLuint stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

Axes::~Axes() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void Axes::draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& MV, const mat4& P) const {
    glUseProgram(_program);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, MV);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, P);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}