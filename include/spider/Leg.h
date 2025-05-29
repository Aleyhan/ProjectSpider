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

        std::vector<vec3> calculateWorldSegmentEnds(const mat4& legModelMatrixAtAttachment) const;
        const std::vector<float>& getJointAngles() const; // Added getter

        // IK related methods
        void solveIKForTarget(const vec3& worldTargetPosition, const mat4& legRootWorldTransform, int iterations = 10, float tolerance = 0.01f);

        // Helper for forward kinematics (can be public if useful elsewhere, or private/protected)
        std::vector<vec3> getAllJointAndTipWorldPositions(const mat4& legRootWorldTransform) const;
        mat4 getJointRotationFrameWorldTransform(int joint_idx, const mat4& legRootWorldTransform) const;

        // Draw the entire leg. modelMatrix should already include the world→root transform.
        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& modelMatrix,
                  const mat4& projMatrix);

        // After draw(), you can query where each segment ended in world space
        const std::vector<vec3>& getSegmentEnds() const;

    private:
        std::vector<float>     jointAngles;
        std::vector<vec3>      segmentEnds;
        float                  thickness;
        std::vector<LegSegment> segments;

    };

} // namespace spider
