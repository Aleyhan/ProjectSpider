#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include <vector>
#include "spider/Eye.h"

namespace spider {

    class Head {
    public:

        explicit Head(GLuint shaderProgram);
        ~Head();

        void draw(GLuint modelViewLoc, GLuint projectionLoc,
                  const mat4& modelMatrix, const mat4& projMatrix) const;

        // Add this in the public section of the Head class
        vec3 getMostFrontVertex() const;

    private:
        GLuint _vao = 0;
        GLuint _vbo = 0;
        GLuint _ebo = 0;
        GLsizei _indexCount = 0;
        GLuint _program = 0;

        std::vector<vec3> _vertexPositions;  // surface points
        std::vector<vec3> _vertexPositionsEye;  // surface points



        // Initializes the mesh
        void initMesh();

        // Generates vertices with normals
        void generateVertices(
            int stacks, int slices,
            float radiusX, float radiusY, float radiusZ,
            std::vector<GLfloat>& interleaved
        );

        // Generates indices for triangle faces
        void generateIndices(
            int stacks, int slices,
            std::vector<GLuint>& indices
        );

        // Uploads mesh data to GPU
        void uploadToGPU(
            const std::vector<GLfloat>& interleaved,
            const std::vector<GLuint>& indices
        );

        // Releases GPU resources
        void cleanup();
    };

} // namespace spider