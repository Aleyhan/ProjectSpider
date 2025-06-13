// Cephalothorax.cpp
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

            float nx = sinV * cosU / radiusX;
            float ny = sinV * sinU / radiusY;
            float nz = cosV / radiusZ;
            vec3 normal = normalize(vec3(nx, ny, nz));

            _vertexPositionsNormal.push_back(vec3(x, y, z));

            float noiseValue = perlin.noise(x * noiseScale, y * noiseScale, z * noiseScale);
            x += normal.x * noiseValue * noiseStrength;
            y += normal.y * noiseValue * noiseStrength;
            z += normal.z * noiseValue * noiseStrength;

            _vertexPositions.emplace_back(x, y, z);

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

            indices.push_back(row1);
            indices.push_back(row2);
            indices.push_back(row1 + 1);

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
    cleanup();

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 interleavedData.size() * sizeof(GLfloat),
                 interleavedData.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(GLuint),
                 indices.data(),
                 GL_STATIC_DRAW);

    constexpr GLuint stride = 6 * sizeof(GLfloat);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                         (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
}

const std::vector<vec3>& Cephalothorax::getVertexPositions() const {
    std::cout << "Cephalothorax vertex positions: " << _vertexPositions.size() << std::endl;
    return _vertexPositions;
}

std::vector<vec3> Cephalothorax::getLegAttachmentPoints() const {
    std::vector<vec3> attachmentPoints;
    attachmentPoints.reserve(8);
    const int points_per_side = 4;

    const float cephBaseRadius = ABDOMEN_RADIUS * 0.8f;
    const float cephRadiusX = cephBaseRadius * ABDOMEN_SCALE_X * 0.7f;
    const float cephRadiusZ = cephBaseRadius * ABDOMEN_SCALE_Z * 1.1f;

    if (_vertexPositionsNormal.empty()) {
        float default_side_x = cephRadiusX * 0.85f;
        float z_attach_min = -cephRadiusZ * 0.8f;
        float z_attach_max =  cephRadiusZ * 0.8f;

        for (int i = 0; i < points_per_side; ++i) {
            float t = (points_per_side <= 1) ? 0.5f : static_cast<float>(i) / (points_per_side - 1);
            float current_z = z_attach_min + t * (z_attach_max - z_attach_min);
            attachmentPoints.emplace_back(-default_side_x, 0.0f, current_z); // Left point
            attachmentPoints.emplace_back( default_side_x, 0.0f, current_z); // Right point
        }
        return attachmentPoints;
    }

    float minX_actual = std::numeric_limits<float>::max();
    float maxX_actual = -std::numeric_limits<float>::max();
    for (const vec3& vertex : _vertexPositionsNormal) {
        if (vertex.x < minX_actual) minX_actual = vertex.x;
        if (vertex.x > maxX_actual) maxX_actual = vertex.x;
    }

    if (minX_actual >= maxX_actual - 0.01f) {
        minX_actual = -cephRadiusX;
        maxX_actual =  cephRadiusX;
    }

    float x_width = maxX_actual - minX_actual;
    float leftThreshold = minX_actual + x_width * 0.20f;
    float rightThreshold = maxX_actual - x_width * 0.20f;


    std::vector<vec3> leftSidePoints;
    std::vector<vec3> rightSidePoints;


    const float filter_z_abs_limit = 0.9f * (ABDOMEN_RADIUS * ABDOMEN_SCALE_Z);

    for (const vec3& vertex : _vertexPositionsNormal) {
        if (std::abs(vertex.z) > filter_z_abs_limit) {
            continue;
        }

        if (vertex.x < leftThreshold) {
            leftSidePoints.push_back(vertex);
        } else if (vertex.x > rightThreshold) {
            rightSidePoints.push_back(vertex);
        }
    }

    auto compareByZ = [](const vec3& a, const vec3& b) { return a.z < b.z; };
    std::sort(leftSidePoints.begin(), leftSidePoints.end(), compareByZ);
    std::sort(rightSidePoints.begin(), rightSidePoints.end(), compareByZ);

    auto findClosestToZ = [](const std::vector<vec3>& sorted_points, float target_z) -> vec3 {
        if (sorted_points.size() == 1) {
            return sorted_points[0];
        }

        auto it = std::lower_bound(sorted_points.begin(), sorted_points.end(), target_z,
                                   [](const vec3& p, float val){ return p.z < val; });

        if (it == sorted_points.begin()) return *it;
        if (it == sorted_points.end()) return sorted_points.back();

        float dist_it = std::abs(it->z - target_z);
        float dist_prev_it = std::abs((it - 1)->z - target_z);
        return (dist_it < dist_prev_it) ? *it : *(it - 1);
    };

    std::vector<vec3> final_left_points(points_per_side);
    std::vector<vec3> final_right_points(points_per_side);

    if (leftSidePoints.empty()) {
        float representative_x = minX_actual * 0.85f; // Default X for left side
         if (representative_x > -0.01f && representative_x < 0.0f) representative_x = -cephRadiusX * 0.85f; // ensure it's reasonably placed if minX_actual was near 0
        for (int i = 0; i < points_per_side; ++i) {
            float t = (points_per_side <= 1) ? 0.5f : static_cast<float>(i) / (points_per_side - 1);
            float z_val = -filter_z_abs_limit + t * (2.0f * filter_z_abs_limit); // Spread along the Z filter range
            final_left_points[i] = vec3(representative_x, 0.0f, z_val);
        }
    } else {
        float z_min = leftSidePoints.front().z;
        float z_max = leftSidePoints.back().z;
        if (z_min >= z_max - 0.01f) { // All points at effectively the same Z
            for (int i = 0; i < points_per_side; ++i) final_left_points[i] = leftSidePoints.front();
        } else {
            for (int i = 0; i < points_per_side; ++i) {
                float t = (points_per_side <= 1) ? 0.5f : static_cast<float>(i) / (points_per_side - 1);
                float target_z = z_min + t * (z_max - z_min);
                final_left_points[i] = findClosestToZ(leftSidePoints, target_z);
            }
        }
    }

    if (rightSidePoints.empty()) {
        float representative_x = maxX_actual * 0.85f; // Default X for right side
        if (representative_x < 0.01f && representative_x > 0.0f) representative_x = cephRadiusX * 0.85f; // ensure it's reasonably placed if maxX_actual was near 0
        for (int i = 0; i < points_per_side; ++i) {
            float t = (points_per_side <= 1) ? 0.5f : static_cast<float>(i) / (points_per_side - 1);
            float z_val = -filter_z_abs_limit + t * (2.0f * filter_z_abs_limit); // Spread along the Z filter range
            final_right_points[i] = vec3(representative_x, 0.0f, z_val);
        }
    } else {
        float z_min = rightSidePoints.front().z;
        float z_max = rightSidePoints.back().z;
         if (z_min >= z_max - 0.01f) {
            for (int i = 0; i < points_per_side; ++i) final_right_points[i] = rightSidePoints.front();
        } else {
            for (int i = 0; i < points_per_side; ++i) {
                float t = (points_per_side <= 1) ? 0.5f : static_cast<float>(i) / (points_per_side - 1);
                float target_z = z_min + t * (z_max - z_min);
                final_right_points[i] = findClosestToZ(rightSidePoints, target_z);
            }
        }
    }

    for (int i = 0; i < points_per_side; ++i) {
        attachmentPoints.emplace_back(final_left_points[i].x, 0.0f, final_left_points[i].z);
        attachmentPoints.emplace_back(final_right_points[i].x, 0.0f, final_right_points[i].z);
    }

    return attachmentPoints;
}

vec3 Cephalothorax::getHeadAnchorPoint() const {
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