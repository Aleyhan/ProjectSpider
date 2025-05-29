// Leg.cpp
 #include "spider/Leg.h"
 #include <algorithm>
#include <cmath>     // For atan2, sqrt, acos, M_PI



 namespace spider {

     Leg::Leg(GLuint shaderProgram,
              int numSegments,
              float segmentLength,
              float segmentThickness)
         : thickness(segmentThickness)
     {
         // Initialize shared geometry if not already done
         LegSegment::initSharedGeometry(shaderProgram);

         segments.reserve(numSegments);
         jointAngles.assign(numSegments, 0.0f);
         segmentEnds.resize(numSegments);

         for (int i = 0; i < numSegments; ++i) {
             // Create segment with the specified length and thickness
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

     std::vector<vec3> Leg::calculateWorldSegmentEnds(const mat4& legModelMatrixAtAttachment) const {
         std::vector<vec3> world_ends;
         world_ends.reserve(segments.size());
         mat4 current_transform = legModelMatrixAtAttachment;

         for (size_t i = 0; i < segments.size(); ++i) {
             current_transform = current_transform * RotateZ(jointAngles[i]);
             // No drawing, just calculate where the segment would be
             mat4 segment_end_transform = current_transform * Translate(segments[i].getLength(), 0.0f, 0.0f);

             vec4 world_end_v4 = segment_end_transform * vec4(0.0f, 0.0f, 0.0f, 1.0f);
             world_ends.emplace_back(world_end_v4.x, world_end_v4.y, world_end_v4.z);

             current_transform = segment_end_transform; // Continue from the end of this segment
         }
         return world_ends;
     }

     std::vector<vec3> Leg::getAllJointAndTipWorldPositions(const mat4& legRootWorldTransform) const {
         std::vector<vec3> positions;
         positions.reserve(segments.size() + 1);

         mat4 current_transform = legRootWorldTransform;
         positions.push_back(vec3(current_transform[0][3], current_transform[1][3], current_transform[2][3]));

         for (size_t i = 0; i < segments.size(); ++i) {
             current_transform = current_transform * RotateZ(jointAngles[i]);
             current_transform = current_transform * Translate(segments[i].getLength(), 0.0f, 0.0f);
             positions.push_back(vec3(current_transform[0][3], current_transform[1][3], current_transform[2][3]));
         }
         return positions;
     }

     mat4 Leg::getJointRotationFrameWorldTransform(int joint_idx, const mat4& legRootWorldTransform) const {
         mat4 current_transform = legRootWorldTransform;
         for (int i = 0; i < joint_idx; ++i) {
             current_transform = current_transform * RotateZ(jointAngles[i]);
             current_transform = current_transform * Translate(segments[i].getLength(), 0.0f, 0.0f);
         }
         return current_transform;
     }

         void Leg::solveIKForTarget(const vec3& worldTargetPosition, const mat4& legRootWorldTransform, int iterations, float tolerance) {
        if (segments.empty()) return;

        // CCD Algorithm
        for (int iter = 0; iter < iterations; ++iter) {
            std::vector<vec3> all_world_points = getAllJointAndTipWorldPositions(legRootWorldTransform);
            vec3 endEffectorWorldPos = all_world_points.back();

            if (length(endEffectorWorldPos - worldTargetPosition) < tolerance) {
                break;
            }

            for (int j = segments.size() - 1; j >= 0; --j) {
                all_world_points = getAllJointAndTipWorldPositions(legRootWorldTransform);
                vec3 currentJointWorldPos = all_world_points[j];
                endEffectorWorldPos = all_world_points.back();

                mat4 jointFrameWorldTransform = getJointRotationFrameWorldTransform(j, legRootWorldTransform);
                mat4 worldToJointFrameTransform = Inverse(jointFrameWorldTransform);

                vec4 endEffectorInJointFrame_v4 = worldToJointFrameTransform * vec4(endEffectorWorldPos, 1.0f);
                vec4 targetInJointFrame_v4 = worldToJointFrameTransform * vec4(worldTargetPosition, 1.0f);

                vec2 vecToEffector = normalize(vec2(endEffectorInJointFrame_v4.x, endEffectorInJointFrame_v4.y));
                vec2 vecToTarget   = normalize(vec2(targetInJointFrame_v4.x, targetInJointFrame_v4.y));

                float cosAngle = dot(vecToEffector, vecToTarget);
                cosAngle = std::max(-1.0f, std::min(1.0f, cosAngle)); // Clamp due to potential precision issues
                float angleRad = acos(cosAngle);

                // Determine sign of angle using 2D cross product (z-component of 3D cross)
                float cross_z = vecToEffector.x * vecToTarget.y - vecToEffector.y * vecToTarget.x;
                if (cross_z < 0.0f) { // If target is clockwise from effector
                    angleRad = -angleRad;
                }

                float angleDeg = Angel::Degrees(angleRad);

                jointAngles[j] += angleDeg;

                // Clamp joint angle
                jointAngles[j] = std::max(-90.0f, std::min(90.0f, jointAngles[j]));
            }
        }
    }

     void Leg::draw(GLuint modelViewLoc,
                    GLuint projectionLoc,
                    const mat4& modelMatrix,
                    const mat4& projMatrix)
     {
         segmentEnds.clear();
         segmentEnds.reserve(segments.size());

         // 'current' is the running transform from the root down the chain
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

     const std::vector<vec3>& Leg::getSegmentEnds() const {
         return segmentEnds;
     }

 } // namespace spider