#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"

class Axes {
public:
    Axes(GLuint program);  // accept program ID from outside
    ~Axes();

    void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& MV, const mat4& P) const;

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
    GLuint _indexCount = 0;
    GLuint _program = 0;
};
