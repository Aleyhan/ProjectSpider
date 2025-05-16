// Leg.h
#pragma once

#include <vector>
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include "LegSegment.h"

namespace spider {

    class Leg {
    public:
        // shaderProgram: shared shader used by all segments
        // numSegments: 7 for a spider
        // segmentLength: default length for each segment
        // segmentThickness: thickness of each segment
        Leg(GLuint shaderProgram,
            int numSegments = 7,
            float segmentLength = 0.4f,
            float segmentThickness = 0.05f);

        // Set the per-joint pitch angle (in degrees) around local X
        void setJointAngles(const std::vector<float>& angles);

        // Draw the entire leg. modelMatrix should already include the world→root transform.
        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& modelMatrix,
                  const mat4& projMatrix);

        // After draw(), you can query where each segment ended in world space
        const std::vector<vec3>& getSegmentEnds() const;
        std::vector<LegSegment> segments;

    private:
        std::vector<float>     jointAngles;
        std::vector<vec3>      segmentEnds;
        float                  thickness;
    };

} // namespace spider
