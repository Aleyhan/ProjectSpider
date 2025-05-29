// LegSegment.cpp
#include "spider/LegSegment.h"
#include <vector>

// ---- static fields ----
GLuint  spider::LegSegment::s_program      = 0;
GLuint  spider::LegSegment::s_vao          = 0;
GLuint  spider::LegSegment::s_vbo          = 0;
GLuint  spider::LegSegment::s_ebo          = 0;
GLsizei spider::LegSegment::s_indexCount   = 0;
bool    spider::LegSegment::s_initialized  = false;

namespace spider {

void LegSegment::initSharedGeometry(GLuint shaderProgram, float canonicalThick)
{
    if (s_initialized) return;
    s_program = shaderProgram;

    // A 1-unit cuboid from x=0 to x=1, centered at y/z = 0 with half-thickness
    const float h = canonicalThick * 0.5f;
    std::vector<GLfloat> verts = {
        // pos (x,y,z)              // no normals needed yet
        0, -h, -h,   1, -h, -h,   1,  h, -h,   0,  h, -h, // back
        0, -h,  h,   1, -h,  h,   1,  h,  h,   0,  h,  h  // front
    };
    std::vector<GLuint> inds = {
        0,1,2, 2,3,0,   // back
        4,5,6, 6,7,4,   // front
        0,4,7, 7,3,0,   // left
        1,5,6, 6,2,1,   // right
        3,2,6, 6,7,3,   // top
        0,1,5, 5,4,0    // bottom
    };
    s_indexCount = GLsizei(inds.size());

    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    glGenBuffers(1, &s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &s_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);

    glBindVertexArray(0);
    s_initialized = true;
}

void LegSegment::cleanupShared() {
    if (!s_initialized) return;
    glDeleteVertexArrays(1, &s_vao);
    glDeleteBuffers(1, &s_vbo);
    glDeleteBuffers(1, &s_ebo);
    s_vao = s_vbo = s_ebo = 0;
    s_initialized = false;
}

// ---- instance methods ----
LegSegment::LegSegment(float length, float thickness)
    : m_length(length), m_thickness(thickness) {}

void LegSegment::setLength(float len)      { m_length = len; }
float LegSegment::getLength()       const  { return m_length; }

void LegSegment::setThickness(float t)     { m_thickness = t; }
float LegSegment::getThickness()    const  { return m_thickness; }

void LegSegment::draw(GLuint mvLoc,
                      GLuint prLoc,
                      const mat4& modelMatrix,
                      const mat4& projMatrix) const
{
    if (!s_initialized) return;

    mat4 M = modelMatrix * Scale(m_length, m_thickness, m_thickness);

    glUseProgram(s_program);
    glUniformMatrix4fv(mvLoc, 1, GL_TRUE, M);    // Angel matrices are row-major
    glUniformMatrix4fv(prLoc, 1, GL_TRUE, projMatrix);

    glBindVertexArray(s_vao);
    glDrawElements(GL_TRIANGLES, s_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace spider
