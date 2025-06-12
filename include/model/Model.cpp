#include <GL/glew.h>
#include "Model.h"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "../external/Angel/inlcude/Angel/Angel.h"
using namespace Angel;

Model::Model(const std::string& path)
    : program(0), vertexCount(0)
{
    std::cerr << "Model(path) constructor is not yet implemented.\n";
}

Model::Model(const std::string& objPath, const std::string& mtlPath, GLuint shaderProgram)
    : program(shaderProgram)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str(), nullptr, true);
    if (!ret) {
        std::cerr << "TinyObjLoader error: " << err << std::endl;
        return;
    }

    std::vector<vec4> vertices;
    std::vector<vec3> normals;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            float vx = attrib.vertices[3 * index.vertex_index + 0];
            float vy = attrib.vertices[3 * index.vertex_index + 1];
            float vz = attrib.vertices[3 * index.vertex_index + 2];
            vertices.push_back(vec4(vx, vy, vz, 1.0f));

            if (!attrib.normals.empty()) {
                float nx = attrib.normals[3 * index.normal_index + 0];
                float ny = attrib.normals[3 * index.normal_index + 1];
                float nz = attrib.normals[3 * index.normal_index + 2];
                normals.push_back(vec3(nx, ny, nz));
            } else {
                normals.push_back(vec3(0.0f, 1.0f, 0.0f));
            }
        }
    }

    vertexCount = vertices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &nbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Model::draw(const mat4& modelView, const mat4& projection) {
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "model_view"), 1, GL_TRUE, modelView);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_TRUE, projection);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}