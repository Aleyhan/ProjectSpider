#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include <vector>

namespace spider {

    class Abdomen {
    public:
        explicit Abdomen(GLuint shaderProgram);
        ~Abdomen();

        // Draws the abdomen using the currently bound shader/program
        void draw(GLuint modelViewLoc, GLuint projectionLoc,
                  const mat4& modelMatrix, const mat4& P) const;

    private:
        GLuint  _vao = 0;
        GLuint  _vbo = 0;
        GLuint  _ebo = 0;
        GLsizei _indexCount = 0;
        GLuint _program = 0;

        // Initializes the entire mesh process
        void initMesh();

        // Generates the vertex positions and normals
        void generateVertices(
            int stacks, int slices,
            float radiusX, float radiusY, float radiusZ,
            std::vector<GLfloat>& interleavedData
        );

        // Generates the indices for triangle faces
        void generateIndices(
            int stacks, int slices,
            std::vector<GLuint>& indices
        );

        // Uploads mesh data to the GPU
        void uploadToGPU(
            const std::vector<GLfloat>& interleavedData,
            const std::vector<GLuint>& indices
        );

        // Releases GPU resources
        void cleanup();
    };

} // namespace spider