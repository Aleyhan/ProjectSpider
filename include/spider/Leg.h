// Leg.h
#pragma once

#include <vector>
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include "LegSegment.h"

namespace spider {

    class Leg {
    public:
        Leg(GLuint shaderProgram,
            int numSegments = 7,
            float segmentLength = 0.4f,
            float segmentThickness = 0.05f);

        // Set the per-joint pitch angle (in degrees) around local X
        void setJointAngles(const std::vector<float>& angles);

        const std::vector<float>& getJointAngles() const; // Added getter


        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& modelMatrix,
                  const mat4& projMatrix);

        const std::vector<vec3>& getSegmentEnds() const;

        std::vector<float> inverseKinematicsCCD(
            float x_target, float y_target, float L, int n, int maxIter, float tol,
            const std::vector<float>& theta_min, const std::vector<float>& theta_max
        );

        void forwardKinematics(const std::vector<float>& theta_deg, float L, std::vector<float>& x, std::vector<float>& y);

    private:
        std::vector<float>     jointAngles;
        std::vector<vec3>      segmentEnds;
        float                  thickness;
        std::vector<LegSegment> segments;

    };

} // namespace spider
