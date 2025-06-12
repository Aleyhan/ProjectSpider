#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"

namespace spider {


    class LegSegment {
    public:
        static void initSharedGeometry(GLuint shaderProgram, float canonicalThickness = 0.05f);
        static void cleanupShared();

        LegSegment(float length = 0.4f, float thickness = 0.05f);

        void   setLength(float len);
        float  getLength()     const;

        void   setThickness(float t);
        float  getThickness()  const;


        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& modelMatrix,
                  const mat4& projMatrix) const;

    private:
        float  m_length;
        float  m_thickness;

        static GLuint s_program;
        static GLuint s_vao;
        static GLuint s_vbo;
        static GLuint s_ebo;
        static GLsizei s_indexCount;
        static bool s_initialized;
    };

} // namespace spider
