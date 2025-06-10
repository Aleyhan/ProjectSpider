#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h> 
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef Error
#undef Error
#endif
#include "../../external/tinyobjloader/tiny_obj_loader.h"
#include "../external/Angel/inlcude/Angel/Angel.h"

using namespace Angel;

class Model {
public:
    Model(const std::string& path);
    Model(const std::string& objPath, const std::string& mtlPath, GLuint shaderProgram);
    void draw(const mat4& modelView, const mat4& projection);

private:
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    GLuint vao, vbo, nbo;
    GLuint shaderProgram;
    GLuint program;
    int vertexCount;
    void loadModel(const std::string& path);
};

#endif // MODEL_H