#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include <vector>

namespace spider {

    class Cephalothorax {
    public:
        explicit Cephalothorax(GLuint shaderProgram);
        ~Cephalothorax();

        void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& modelMatrix, const mat4& P) const;

        // Getters for leg attachment points and head anchor point
        const std::vector<vec3>& getVertexPositions() const;
        std::vector<vec3> getLegAttachmentPoints() const;
        vec3 getHeadAnchorPoint() const;

    private:
        void initMesh();
        void cleanup();

        // Mesh generation helper methods
        void generateVertexData(int stacks, int slices,
                              float radiusX, float radiusY, float radiusZ,
                              std::vector<GLfloat>& interleavedData);
        void generateIndices(int stacks, int slices, std::vector<GLuint>& indices);
        void uploadToGPU(const std::vector<GLfloat>& interleavedData,
                        const std::vector<GLuint>& indices);

        std::vector<vec3> _vertexPositions; // only positions, not normals
        std::vector<vec3> _vertexPositionsNormal; // only positions, not normals


        GLuint _program;
        GLuint _vao;
        GLuint _vbo;
        GLuint _ebo;
        GLsizei _indexCount;
    };

} // namespace spider