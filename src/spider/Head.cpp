#include "spider/Head.h"
#include "global/GlobalConfig.h"
#include <vector>
#include <cmath>
#include "utils/PerlinNoise.h"
#include <algorithm>
#include "spider/Eye.h"


namespace spider {

namespace {
    constexpr int DEFAULT_STACKS = 30;
    constexpr int DEFAULT_SLICES = 30;
}

Head::Head(GLuint shaderProgram)
    : _program(shaderProgram) {
    initMesh();

}

Head::~Head() {
    cleanup();
}

void Head::cleanup() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        _vao = _vbo = _ebo = 0;
    }
}

void Head::initMesh() {
    _vertexPositions.clear();
    std::vector<GLfloat> interleavedVertices;
    std::vector<GLuint> indices;

    const float baseRadius = ABDOMEN_RADIUS * HEAD_SCALE;
    const float radiusX = baseRadius;
    const float radiusY = baseRadius;
    const float radiusZ = baseRadius * HEAD_SCALE_Z;

    generateVertices(DEFAULT_STACKS, DEFAULT_SLICES, radiusX, radiusY, radiusZ, interleavedVertices);
    generateIndices(DEFAULT_STACKS, DEFAULT_SLICES, indices);
    uploadToGPU(interleavedVertices, indices);
}


void Head::generateVertices(
    int stacks, int slices,
    float radiusX, float radiusY, float radiusZ,
    std::vector<GLfloat>& interleavedVertices
) {
    PerlinNoise perlin;
    const float noiseFrequency = NOISE_SCALE * 0.45f;
    const float noiseAmplitude = NOISE_STRENGHT * ABDOMEN_RADIUS * 0.5f;

    for (int i = 0; i <= stacks; ++i) {
        float v = M_PI * i / stacks;
        float sinV = std::sin(v);
        float cosV = std::cos(v);

        for (int j = 0; j <= slices; ++j) {
            float u = 2.0f * M_PI * j / slices;
            float sinU = std::sin(u);
            float cosU = std::cos(u);

            float x = radiusX * sinV * cosU;
            float y = radiusY * sinV * sinU;
            float z = radiusZ * cosV;

            // Calculate normal
            vec3 normal = normalize(vec3(sinV * cosU / radiusX, sinV * sinU / radiusY, cosV / radiusZ));
            _vertexPositionsEye.emplace_back(x, y, z);

            // Apply noise displacement
            float noiseValue = perlin.noise(x * noiseFrequency, y * noiseFrequency, z * noiseFrequency);
            x += normal.x * noiseValue * noiseAmplitude;
            y += normal.y * noiseValue * noiseAmplitude;
            z += normal.z * noiseValue * noiseAmplitude;

            _vertexPositions.emplace_back(x, y, z);
            interleavedVertices.insert(interleavedVertices.end(), {x, y, z, normal.x, normal.y, normal.z});
        }
    }
}

void Head::generateIndices(
    int stacks, int slices,
    std::vector<GLuint>& indices
) {
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            GLuint row1 = i * (slices + 1) + j;
            GLuint row2 = row1 + (slices + 1);

            indices.push_back(row1);
            indices.push_back(row2);
            indices.push_back(row1 + 1);

            indices.push_back(row2);
            indices.push_back(row2 + 1);
            indices.push_back(row1 + 1);
        }
    }
    _indexCount = static_cast<GLsizei>(indices.size());
}

    vec3 Head::getMostFrontVertex() const {
    if (_vertexPositionsEye.empty()) {
        return vec3(0.0f, 0.0f, 0.0f);
    }

    // Still use raw vertices as requested
    vec3 maxZVertex = _vertexPositionsEye[0];

    for (const auto& vertex : _vertexPositionsEye) {
        if (vertex.z > maxZVertex.z) {
            maxZVertex = vertex;
        }
    }

    // Apply a small offset to avoid exact coordinate values that might trigger axes
    // Using 0.001 to minimize visual impact but still avoid the axes
    return maxZVertex + vec3(0.001f, 0.001f, 0.001f);
}

void Head::uploadToGPU(
    const std::vector<GLfloat>& interleavedVertices,
    const std::vector<GLuint>& indices
) {
    cleanup(); // Ensure previous buffers are cleaned up

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, interleavedVertices.size() * sizeof(GLfloat), interleavedVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    constexpr GLuint stride = 6 * sizeof(GLfloat);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

    void Head::draw(GLuint modelViewLoc, GLuint projectionLoc,
                    const mat4& modelMatrix, const mat4& projMatrix) const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Add this for safety

    glUseProgram(_program);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projMatrix);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

}

} // namespace spider