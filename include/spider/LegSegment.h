#pragma once
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"

namespace spider {

    /**
     * Canonical cuboid-based leg segment.
     *  - Geometry: 1-unit long along +X, centered in Y/Z
     *  - Drawn via a modelMatrix supplied by the Leg class
     *  - Geometry data is static/shared for every LegSegment instance
     */
    class LegSegment {
    public:
        /// Call once (e.g., from Spider constructor) to allocate the shared mesh.
        static void initSharedGeometry(GLuint shaderProgram, float canonicalThickness = 0.05f);
        /// Call once at shutdown to free GPU buffers.
        static void cleanupShared();

        /// Construct an instance that will be drawn with its own length/thickness scale.
        LegSegment(float length = 0.4f, float thickness = 0.05f);

        // --- size setters / getters ---
        void   setLength(float len);
        float  getLength()     const;

        void   setThickness(float t);
        float  getThickness()  const;

        /**
         * Draw the segment.
         *  - modelMatrix must position the *base* at the desired world space,
         *    then any joint rotation, then we scale by (length, thickness, thickness)
         *    INSIDE this function.
         */
        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& modelMatrix,
                  const mat4& projMatrix) const;

    private:
        float  m_length;
        float  m_thickness;

        // -------- static shared GPU data --------
        static GLuint s_program;
        static GLuint s_vao;
        static GLuint s_vbo;
        static GLuint s_ebo;
        static GLsizei s_indexCount;
        static bool s_initialized;
    };

} // namespace spider
