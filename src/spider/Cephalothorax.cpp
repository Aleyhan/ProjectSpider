#include "spider/Cephalothorax.h"
#include "global/GlobalConfig.h"
#include "utils/PerlinNoise.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

namespace spider {

Cephalothorax::Cephalothorax(GLuint shaderProgram)
    : _program(shaderProgram), _vao(0), _vbo(0), _ebo(0), _indexCount(0) {
    initMesh();
}

Cephalothorax::~Cephalothorax() {
    cleanup();
}

void Cephalothorax::cleanup() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        _vao = _vbo = _ebo = 0;
    }
}

void Cephalothorax::initMesh() {
    const int stacks = 30;
    const int slices = 30;

    const float baseRadius = ABDOMEN_RADIUS *0.8f;
    const float radiusX = baseRadius * ABDOMEN_SCALE_X*0.7F;
    const float radiusY = baseRadius * ABDOMEN_SCALE_X * 0.7F;
    const float radiusZ = baseRadius * ABDOMEN_SCALE_Z * 1.1F;

    std::vector<GLfloat> interleavedData;
    std::vector<GLuint> indices;
    _vertexPositions.clear();
    _vertexPositionsNormal.clear();

    generateVertexData(stacks, slices, radiusX, radiusY, radiusZ, interleavedData);
    generateIndices(stacks, slices, indices);
    uploadToGPU(interleavedData, indices);
}

void Cephalothorax::generateVertexData(
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

            _vertexPositionsNormal.push_back(vec3(x, y, z));

            // Apply noise-based displacement
            float noiseValue = perlin.noise(x * noiseScale, y * noiseScale, z * noiseScale);
            x += normal.x * noiseValue * noiseStrength;
            y += normal.y * noiseValue * noiseStrength;
            z += normal.z * noiseValue * noiseStrength;

            // Store vertex position for later use
            _vertexPositions.emplace_back(x, y, z);

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

void Cephalothorax::generateIndices(
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

void Cephalothorax::uploadToGPU(
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

const std::vector<vec3>& Cephalothorax::getVertexPositions() const {
    std::cout << "Cephalothorax vertex positions: " << _vertexPositions.size() << std::endl;
    return _vertexPositions;
}

std::vector<vec3> Cephalothorax::getLegAttachmentPoints() const {
    std::vector<vec3> leftSidePoints;
    std::vector<vec3> rightSidePoints;
    std::vector<vec3> attachmentPoints;

    // Find extreme X values (keep this part the same)
    float maxX = -std::numeric_limits<float>::max();
    float minX = std::numeric_limits<float>::max();

    for (const vec3& vertex : _vertexPositionsNormal) {
        if (vertex.x > maxX) maxX = vertex.x;
        if (vertex.x < minX) minX = vertex.x;
    }

    // Adjust threshold to move points more inward if needed
    float leftThreshold = minX * 0.85f;
    float rightThreshold = maxX * 0.85f;

    // Modify this Z filtering to increase range
    // Previously was 0.8f - make this smaller to allow more points
    for (const vec3& vertex : _vertexPositionsNormal) {
        // Use 0.9f instead of 0.8f to include more points toward front/back
        if (vertex.z < -0.9f * ABDOMEN_RADIUS * ABDOMEN_SCALE_Z ||
            vertex.z > 0.9f * ABDOMEN_RADIUS * ABDOMEN_SCALE_Z) {
            continue;
        }

        if (vertex.x < leftThreshold) {
            leftSidePoints.push_back(vertex);
        } else if (vertex.x > rightThreshold) {
            rightSidePoints.push_back(vertex);
        }
    }

    // Sort by Z and get min/max values (keep this part)
    auto compareByZ = [](const vec3& a, const vec3& b) { return a.z < b.z; };
    std::sort(leftSidePoints.begin(), leftSidePoints.end(), compareByZ);
    std::sort(rightSidePoints.begin(), rightSidePoints.end(), compareByZ);

    if (leftSidePoints.empty() || rightSidePoints.empty()) {
        return attachmentPoints;
    }

    float leftMinZ = leftSidePoints.front().z;
    float leftMaxZ = leftSidePoints.back().z;
    float rightMinZ = rightSidePoints.front().z;
    float rightMaxZ = rightSidePoints.back().z;

    // Add a spread factor to extend the range beyond min/max
    float spreadFactor = 2.0f;  // Increase this to spread legs further apart

    // Calculate the center of the Z range
    float leftZCenter = (leftMaxZ + leftMinZ) / 2.0f;
    float rightZCenter = (rightMaxZ + rightMinZ) / 2.0f;

    // Calculate the half range and extend it by the spread factor
    float leftZRange = (leftMaxZ - leftMinZ) / 2.0f * spreadFactor;
    float rightZRange = (rightMaxZ - rightMinZ) / 2.0f * spreadFactor;

    // Calculate new min/max values
    float leftExtendedMinZ = leftZCenter - leftZRange;
    float leftExtendedMaxZ = leftZCenter + leftZRange;
    float rightExtendedMinZ = rightZCenter - rightZRange;
    float rightExtendedMaxZ = rightZCenter + rightZRange;

    // Use extended range for spacing calculations
    // After calculating the extended Z ranges, add this code:
    for (int i = 0; i < 4; i++) {
        float leftSpacing = leftExtendedMinZ + (leftExtendedMaxZ - leftExtendedMinZ) * i / 2.0f;
        float rightSpacing = rightExtendedMinZ + (rightExtendedMaxZ - rightExtendedMinZ) * i / 2.0f;

        // Find closest points to these spacings as before
        vec3 leftPoint = leftSidePoints[0];
        vec3 rightPoint = rightSidePoints[0];

        for (const vec3& point : leftSidePoints) {
            if (std::abs(point.z - leftSpacing) < std::abs(leftPoint.z - leftSpacing)) {
                leftPoint = point;
            }
        }

        for (const vec3& point : rightSidePoints) {
            if (std::abs(point.z - rightSpacing) < std::abs(rightPoint.z - rightSpacing)) {
                rightPoint = point;
            }
        }

        // Average the Y values between corresponding points
        float avgY = (leftPoint.y + rightPoint.y) / 2.0f;

        // Create adjusted points with the same Y coordinate
        vec3 adjustedLeftPoint(leftPoint.x, 0, leftPoint.z);
        vec3 adjustedRightPoint(rightPoint.x, 0, rightPoint.z);

        attachmentPoints.push_back(adjustedLeftPoint);
        attachmentPoints.push_back(adjustedRightPoint);
    }

    return attachmentPoints;
}

vec3 Cephalothorax::getHeadAnchorPoint() const {
    // Find the vertex with the minimum Z value (most forward point)
    vec3 headPoint(0.0f, 0.0f, 0.0f);
    float maxZ = -std::numeric_limits<float>::max();

    for (const vec3& vertex : _vertexPositionsNormal) {
        if (vertex.z > maxZ) {
            maxZ = vertex.z;
            headPoint = vertex;
        }
    }

    return headPoint;
}

void Cephalothorax::draw(GLuint modelViewLoc, GLuint projectionLoc,
                         const mat4& modelMatrix, const mat4& P) const {
    glUseProgram(_program);
    glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, P);

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace spider