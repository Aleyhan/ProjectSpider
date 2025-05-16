// Leg.cpp
             #include "spider/Leg.h"
             #include <algorithm>

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