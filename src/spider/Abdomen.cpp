#include "spider/Abdomen.h"
#include "global/GlobalConfig.h"
#include <vector>
#include <cmath>
#include "utils/PerlinNoise.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace spider {

Abdomen::Abdomen(GLuint shaderProgram)
    : _program(shaderProgram) {
    initMesh();
}

Abdomen::~Abdomen() {
    cleanup();
}

void Abdomen::cleanup() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        _vao = 0;
        _vbo = 0;
        _ebo = 0;
    }
}

void Abdomen::initMesh() {
    const int stacks = 30;
    const int slices = 30;
    const float radiusX = ABDOMEN_RADIUS * ABDOMEN_SCALE_X;
    const float radiusY = ABDOMEN_RADIUS;
    const float radiusZ = ABDOMEN_RADIUS * ABDOMEN_SCALE_Z;

    std::vector<GLfloat> interleavedData;
    std::vector<GLuint> indices;

    generateVertices(stacks, slices, radiusX, radiusY, radiusZ, interleavedData);
    generateIndices(stacks, slices, indices);
    uploadToGPU(interleavedData, indices);
}

void Abdomen::generateVertices(
    int stacks, int slices,
    float radiusX, float radiusY, float radiusZ,
    std::vector<GLfloat>& interleavedData
) {
    PerlinNoise perlin;
    float noiseScale = NOISE_SCALE;
    float noiseStrength = NOISE_STRENGHT * ABDOMEN_RADIUS;

    for (int i = 0; i <= stacks; ++i) {
        float v = M_PI * i / stacks;
        float sinV = std::sin(v), cosV = std::cos(v);

        for (int j = 0; j <= slices; ++j) {
            float u = 2.0f * M_PI * j / slices;
            float sinU = std::sin(u), cosU = std::cos(u);

            float x = radiusX * sinV * cosU;
            float y = radiusY * sinV * sinU;
            float z = radiusZ * cosV;

            // Calculate normal before displacement for accurate displacement along normal
            float nx = sinV * cosU / radiusX;
            float ny = sinV * sinU / radiusY;
            float nz = cosV / radiusZ;
            vec3 normal = normalize(vec3(nx, ny, nz));

            // Apply noise-based displacement
            float noiseValue = perlin.noise(x * noiseScale, y * noiseScale, z * noiseScale);
            x += normal.x * noiseValue * noiseStrength;
            y += normal.y * noiseValue * noiseStrength;
            z += normal.z * noiseValue * noiseStrength;

            // Add interleaved vertex data (position + normal)
            interleavedData.push_back(x);
            interleavedData.push_back(y);
            interleavedData.push_back(z);
            interleavedData.push_back(normal.x);
            interleavedData.push_back(normal.y);
            interleavedData.push_back(normal.z);
        }
    }
}

void Abdomen::generateIndices(
    int stacks, int slices,
    std::vector<GLuint>& indices
) {
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            GLuint row1 = i * (slices + 1) + j;
            GLuint row2 = row1 + (slices + 1);

            // First triangle
            indices.push_back(row1);
            indices.push_back(row2);
            indices.push_back(row1 + 1);

            // Second triangle
            indices.push_back(row2);
            indices.push_back(row2 + 1);
            indices.push_back(row1 + 1);
        }
    }

    _indexCount = GLsizei(indices.size());
}

void Abdomen::uploadToGPU(
    const std::vector<GLfloat>& interleavedData,
    const std::vector<GLuint>& indices
) {
    // Clean up previous buffers if they exist
    cleanup();

    // Generate new buffers
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 interleavedData.size() * sizeof(GLfloat),
                 interleavedData.data(),
                 GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(GLuint),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Set attribute pointers
    constexpr GLuint stride = 6 * sizeof(GLfloat);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                         (void*)(3 * sizeof(GLfloat)));

    // Unbind VAO
    glBindVertexArray(0);
}

void Abdomen::draw(GLuint modelViewLoc, GLuint projectionLoc,
                  const mat4& modelMatrix, const mat4& P) const {
    glUseProgram(_program);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, P);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace spider