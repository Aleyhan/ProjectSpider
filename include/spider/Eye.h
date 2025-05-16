#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"

namespace spider {

    class Eye {
    public:
        Eye(GLuint shaderProgram);
        ~Eye();

        void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& modelMatrix, const mat4& projMatrix) const;

    private:
        void initMesh();
        void cleanup();

        GLuint _vao = 0;
        GLuint _vbo = 0;
        GLuint _ebo = 0;
        GLsizei _indexCount = 0;
        GLuint _program = 0;
    };

}
