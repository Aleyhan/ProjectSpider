// Leg.cpp
#include "spider/Leg.h"
#include <cmath>     // For atan2, sqrt, acos, M_PI
#include <vector>

namespace spider {

    Leg::Leg(GLuint shaderProgram,
             int numSegments,
             float segmentLength,
             float segmentThickness)
        : thickness(segmentThickness)
    {
        LegSegment::initSharedGeometry(shaderProgram);

        segments.reserve(numSegments);
        jointAngles.assign(numSegments, 0.0f);
        segmentEnds.resize(numSegments);

        for (int i = 0; i < numSegments; ++i) {
            segments.emplace_back(segmentLength, segmentThickness);
        }
    }

    void Leg::setJointAngles(const std::vector<float>& angles) {
        int n = std::min((int)angles.size(), (int)jointAngles.size());
        for (int i = 0; i < n; ++i) {
            jointAngles[i] = angles[i];
        }
    }

    const std::vector<float>& Leg::getJointAngles() const {
        return jointAngles;
    }

    const std::vector<vec3>& Leg::getSegmentEnds() const {
        return segmentEnds;
    }
    // Applied from matlab (which is written by us inspired from "NUMERICAL METHODS FOR MECHANICAL ENGINEERING-01 (MECH307”))by help of gemini.
    void Leg::forwardKinematics(const std::vector<float>& theta_deg, float L, std::vector<float>& x, std::vector<float>& y) {
        x.assign(theta_deg.size() + 1, 0.0f);
        y.assign(theta_deg.size() + 1, 0.0f);
        float a = 0.0f;
        for (size_t i = 0; i < theta_deg.size(); ++i) {
            a += theta_deg[i];
            float rad = a * static_cast<float>(M_PI) / 180.0f;
            x[i + 1] = x[i] + L * std::cos(rad);
            y[i + 1] = y[i] + L * std::sin(rad);
        }
    }

    std::vector<float> Leg::inverseKinematicsCCD( // Applied from matlab (which is written by us inspired from "NUMERICAL METHODS FOR MECHANICAL ENGINEERING-01 (MECH307”))by help of gemini.
        float x_target, float y_target, float L, int n, int maxIter, float tol,
        const std::vector<float>& theta_min, const std::vector<float>& theta_max
    ) {
        std::vector<float> theta_deg(n, 0.0f);
        std::vector<float> x, y;
        for (int iter = 0; iter < maxIter; ++iter) {
            forwardKinematics(theta_deg, L, x, y);
            float dx = x.back() - x_target;
            float dy = y.back() - y_target;
            float d = std::sqrt(dx*dx + dy*dy);
            if (d < tol) break;
            for (int i = n-1; i >= 0; --i) {
                forwardKinematics(theta_deg, L, x, y);
                float jx = x[i], jy = y[i];
                float eex = x.back(), eey = y.back();
                float v1x = x_target - jx, v1y = y_target - jy;
                float v2x = eex - jx,    v2y = eey - jy;
                float a1 = std::atan2(v1y, v1x) * 180.0f / static_cast<float>(M_PI);
                float a2 = std::atan2(v2y, v2x) * 180.0f / static_cast<float>(M_PI);
                float da = a1 - a2;
                theta_deg[i] += da;
                // Clamp to limits
                theta_deg[i] = std::min(std::max(theta_deg[i], theta_min[i]), theta_max[i]);
            }
        }
        return theta_deg;
    }

     void Leg::draw(GLuint modelViewLoc,
                    GLuint projectionLoc,
                    const mat4& modelMatrix,
                    const mat4& projMatrix)
     {
         segmentEnds.clear();
         segmentEnds.reserve(segments.size());

         mat4 current = modelMatrix;

         for (int i = 0; i < (int)segments.size(); ++i) {
             // 1) Rotate around local X by this joint's angle
             current = current * RotateZ(jointAngles[i]);

             // 2) Draw this segment
             segments[i].draw(modelViewLoc, projectionLoc, current, projMatrix);

             // 3) Advance to the end of this segment so next one attaches there
             current = current * Translate(segments[i].getLength(), 0.0f, 0.0f);

             // 4) Record the world-space end point
             vec4 worldEnd = current * vec4(0.0f, 0.0f, 0.0f, 1.0f);
             segmentEnds.emplace_back(worldEnd.x, worldEnd.y, worldEnd.z);
         }
     }

 } // namespace spider
