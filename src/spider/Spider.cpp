// Description: Source file for the Spider class, implementing its behavior and rendering.
#include "spider/Spider.h"
#include "global/GlobalConfig.h"
#include "spider/Leg.h"
#include <cmath> // For M_PI, sin, cos, fmod


namespace spider {

    Spider::Spider(GLuint cephalothoraxShader, GLuint abdomenShader, GLuint legShader, GLuint eyeShader)
    : cephalothorax(cephalothoraxShader),
      abdomen(abdomenShader),
      head(cephalothoraxShader),
      leftEye(eyeShader),
      rightEye(eyeShader),
      leftEye2(eyeShader),
      rightEye2(eyeShader),
      leg(legShader, 7, 0.6f, 1.3f),
      leg2(legShader, 7, 0.6f, 1.3f),
      leg3(legShader, 7, 0.6f, 1.5f),
      leg4(legShader, 7, 0.6f, 1.5f),
      leg5(legShader, 7, 0.6f, 1.3f),
      leg6(legShader, 7, 0.6f, 1.3f),
      leg7(legShader, 7, 0.6f, 1.5f),
      leg8(legShader, 7, 0.6f, 1.5f),
      position(BODY_START_X, BODY_START_Y, BODY_START_Z),
      current_forward_vector_(0.0f, 0.0f, 1.0f),
      is_walking_forward_(false),
      is_walking_backward_(false),
      is_turning_left_(false),
      is_turning_right_(false),
      walk_speed_(1.5f),
      turn_speed_(90.0f),
      current_yaw_angle_(0.0f),
      leg_animation_cycle_(0.0f),
      leg_animation_speed_(1.4f),
      max_leg_swing_angle_(15.0f),
      base_abdomen_tilt_angle_(ABDOMEN_TILT_ANGLE),
      abdomen_shake_cycle_(0.0f),
      abdomen_shake_speed_(3.0f),
      max_abdomen_shake_amplitude_(5.0f) {

        std::vector<float> angles = {20.0f, 5.0f, -10.0f, -35.0f, -20.0f, -30.0f, -10.0f};
        leg.setJointAngles(angles);
        leg2.setJointAngles(angles);
        leg3.setJointAngles(angles);
        leg4.setJointAngles(angles);
        leg5.setJointAngles(angles);
        leg6.setJointAngles(angles);
        leg7.setJointAngles(angles);
        leg8.setJointAngles(angles);

    }



void Spider::setPosition(const vec3& pos) {
    position = pos;
}

const vec3& Spider::getPosition() const {
    return position;
}

void Spider::startWalkingForward() {
    is_walking_forward_ = true;
    is_walking_backward_ = false;
}

void Spider::stopWalkingForward() {
    is_walking_forward_ = false;
}

void Spider::startWalkingBackward() {
    is_walking_backward_ = true;
    is_walking_forward_ = false;
}

void Spider::stopWalkingBackward() {
    is_walking_backward_ = false;
}

void Spider::startTurningLeft() {
    is_turning_left_ = true;
    is_turning_right_ = false;
}

void Spider::stopTurningLeft() {
    is_turning_left_ = false;
}

void Spider::startTurningRight() {
    is_turning_right_ = true;
    is_turning_left_ = false;
}

void Spider::stopTurningRight() {
    is_turning_right_ = false;
}

std::vector<std::pair<float, float>> Spider::getXYLengthsForAllAttachments(const std::vector<vec3>& attachPoints) {
    std::vector<std::pair<float, float>> results;
    results.reserve(attachPoints.size());
    float dx = 3.0f; // Adjust as needed

    for (const auto& pt : attachPoints) {
        float dy = pt.y - position.y; // Include the spider's vertical position
        results.emplace_back(dx, dy);
    }
    return results;
}

void Spider::applyIKToAllLegs(
    std::vector<spider::Leg*>& legs,
    const std::vector<vec3>& attachPoints,
    float segmentLength,
    int numSegments,
    int maxIter,
    float tol,
    const std::vector<std::vector<float>>& theta_min_all,
    const std::vector<std::vector<float>>& theta_max_all
) {

    auto xyTargets = getXYLengthsForAllAttachments(attachPoints);
    for (size_t i = 0; i < legs.size(); ++i) {
        // Adjust the y-coordinate dynamically for each attachPoint
        float x_target = xyTargets[i].first;
        float y_target = xyTargets[i].second;
        const auto& theta_min = theta_min_all[i];
        const auto& theta_max = theta_max_all[i];
        std::vector<float> angles = legs[i]->inverseKinematicsCCD(
            x_target, y_target, segmentLength, numSegments, maxIter, tol, theta_min, theta_max
        );

        legs[i]->setJointAngles(angles);
    }
}

void Spider::moveBodyUp() {
    position.y += 0.05f; // Adjust this value as needed
        if (position.y > 2.0f) {
            position.y = 2.0f;
        }
    // Update leg positions with IK
    std::vector<spider::Leg*> legs = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};
    std::vector<vec3> attachPoints = cephalothorax.getLegAttachmentPoints();
    float segmentLength = 0.6f;
    int numSegments = 7;
    int maxIter = 10;
    float tol = 0.01f;

    std::vector<std::vector<float>> theta_min_all(legs.size(), {30.0f, -15.0f, -40.0f, -30.0f, -30.0f, -30.0f, -30.0f});
    std::vector<std::vector<float>> theta_max_all(legs.size(), {90.0f, 15.0f, 40.0f, 40.0f, 0.0f, 0.0f, 0.0f});

    applyIKToAllLegs(legs, attachPoints, segmentLength, numSegments, maxIter, tol, theta_min_all, theta_max_all);
}

void Spider::moveBodyDown() {
    position.y -= 0.05f; // Adjust this value as needed
        if (position.y < 0.30f) {
            position.y = 0.30f;
        }
    // Update leg positions with IK
    std::vector<spider::Leg*> legs = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};
    std::vector<vec3> attachPoints = cephalothorax.getLegAttachmentPoints();
    float segmentLength = 0.6f;
    int numSegments = 7;
    int maxIter = 10;
    float tol = 0.01f;

    std::vector<std::vector<float>> theta_min_all(legs.size(), {30.0f, -15.0f, -40.0f, -30.0f, -30.0f, -30.0f, -30.0f});
    std::vector<std::vector<float>> theta_max_all(legs.size(), {90.0f, 15.0f, 40.0f, 40.0f, 0.0f, 0.0f, 0.0f});

    applyIKToAllLegs(legs, attachPoints, segmentLength, numSegments, maxIter, tol, theta_min_all, theta_max_all);
}

void Spider::jump(float deltaTime, float jumpDuration) {
    static float jumpTime = 0.0f;
    static bool isJumping = false;

    if (!isJumping) {
        isJumping = true; // Start the jump
        jumpTime = 0.0f;  // Reset jump time
    }

    if (isJumping) {
        if (jumpTime <= jumpDuration) {
            // Calculate height using a sine wave for smooth animation
            float jumpHeight = 2.0f * sin((jumpTime / jumpDuration) * M_PI); // Adjust 0.5f for max height
            position.y = BODY_START_Y + jumpHeight;

            // Update leg positions with IK
            std::vector<spider::Leg*> legs = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};
            std::vector<vec3> attachPoints = cephalothorax.getLegAttachmentPoints();
            float segmentLength = 0.6f;
            int numSegments = 7;
            int maxIter = 10;
            float tol = 0.01f;

            std::vector<std::vector<float>> theta_min_all(legs.size(), {30.0f, -15.0f, -40.0f, -30.0f, -30.0f, -30.0f, -30.0f});
            std::vector<std::vector<float>> theta_max_all(legs.size(), {90.0f, 15.0f, 40.0f, 40.0f, 0.0f, 0.0f, 0.0f});

            applyIKToAllLegs(legs, attachPoints, segmentLength, numSegments, maxIter, tol, theta_min_all, theta_max_all);

            jumpTime += deltaTime;
        } else {
            // Reset after jump
            position.y = BODY_START_Y; // Reset to starting height
            isJumping = false;
        }
    }
}

void Spider::update(float deltaTime) {
    if (is_turning_left_) {
        current_yaw_angle_ += turn_speed_ * deltaTime;
    }
    if (is_turning_right_) {
        current_yaw_angle_ -= turn_speed_ * deltaTime;
    }
    current_yaw_angle_ = fmod(fmod(current_yaw_angle_, 360.0f) + 360.0f, 360.0f);

    auto to_radians = [](float degrees) {
        return degrees * (static_cast<float>(M_PI) / 180.0f);
    };
    float yaw_rad = to_radians(current_yaw_angle_);
    current_forward_vector_ = normalize(vec3(sin(yaw_rad), 0.0f, cos(yaw_rad)));

    if (is_walking_forward_) {
        position += current_forward_vector_ * walk_speed_ * deltaTime;
    } else if (is_walking_backward_) {
        position -= current_forward_vector_ * walk_speed_ * deltaTime;
    }

    // Prepare data for applyIKToAllLegs
    std::vector<spider::Leg*> legs = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};
    std::vector<vec3> attachPoints = cephalothorax.getLegAttachmentPoints();
    float segmentLength = 0.6f;
    int numSegments = 7;
    int maxIter = 10;
    float tol = 0.01f;

    std::vector<std::vector<float>> theta_min_all(legs.size(), {30.0f, -15.0f, -40.0f, -30.0f, -30.0f, -30.0f, -30.0f});
    std::vector<std::vector<float>> theta_max_all(legs.size(), {90.0f, 15.0f, 40.0f, 40.0f, 0.0f, 0.0f, 0.0f});

    // Call applyIKToAllLegs
    applyIKToAllLegs(legs, attachPoints, segmentLength, numSegments, maxIter, tol, theta_min_all, theta_max_all);

    bool is_active = is_walking_forward_ || is_walking_backward_ || is_turning_left_ || is_turning_right_;
    if (is_active) {
        leg_animation_cycle_ += leg_animation_speed_ * deltaTime;
        if (leg_animation_cycle_ > 1.0f) {
            leg_animation_cycle_ -= 2.0f;
        }

        abdomen_shake_cycle_ += abdomen_shake_speed_ * deltaTime;
        if (abdomen_shake_cycle_ > 1.0f) {
            abdomen_shake_cycle_ -= 1.0f;
        }
    }

        if (jumpTriggered) {
            jump(deltaTime, 1.0f); // 1.0f is the jump duration
            if (getPosition().y == BODY_START_Y) { // Check if the jump is complete
                jumpTriggered = false; // Reset the trigger
            }
        }

}

        void Spider::draw(
            GLuint modelViewLoc,
            GLuint projectionLoc,
            const mat4& viewMatrix,
            const mat4& projMatrix
        ) {
        mat4 R_yaw = Angel::RotateY(current_yaw_angle_);
        mat4 T_translation = Angel::Translate(position);
        mat4 spiderWorldTransform = T_translation * R_yaw;

        mat4 V = viewMatrix;

        mat4 modelCT_View = V * spiderWorldTransform;
        cephalothorax.draw(modelViewLoc, projectionLoc, modelCT_View, projMatrix);

        const float rz_abdomen = ABDOMEN_RADIUS * ABDOMEN_SCALE_Z;
        float current_abdomen_tilt = base_abdomen_tilt_angle_;
        // Abdomen shake is always calculated based on abdomen_shake_cycle_
        // which pauses if spider is not active
        float shake_offset = sin(abdomen_shake_cycle_ * 2.0f * static_cast<float>(M_PI)) * max_abdomen_shake_amplitude_;
        current_abdomen_tilt += shake_offset;

        mat4 R_tilt_ab    = Angel::RotateX(current_abdomen_tilt);
        mat4 T_pivot_ab   = Angel::Translate(0, 0, -rz_abdomen*1.8f);
        mat4 abdomenLocalToParent = R_tilt_ab * T_pivot_ab;
        mat4 modelAbdomen_View = V * spiderWorldTransform * abdomenLocalToParent;
        abdomen.draw(modelViewLoc, projectionLoc, modelAbdomen_View, projMatrix);

        vec3 localHeadPos = cephalothorax.getHeadAnchorPoint();
        mat4 headLocalToCeph = Angel::Translate(localHeadPos);
        mat4 modelHead_World = spiderWorldTransform * headLocalToCeph;
        mat4 modelHead_View = V * modelHead_World;
        head.draw(modelViewLoc, projectionLoc, modelHead_View, projMatrix);

        vec3 headAnchor = head.getMostFrontVertex();
        float scaleFactor = ABDOMEN_RADIUS*HEAD_SCALE / (DEFAULT_ABDOMEN_RADIUS*0.5);
        vec3 leftOffset  = vec3(-0.07f*scaleFactor, +0.05f*scaleFactor, 0.1f*scaleFactor);
        vec3 rightOffset = vec3(+0.07f*scaleFactor, +0.05f*scaleFactor, 0.1f*scaleFactor);
        float zElongation = 1.2f;

        mat4 modelLeftEye_View  = V * modelHead_World * Angel::Translate(headAnchor + leftOffset)*Angel::Scale(1.0f, 1.0f, zElongation);
        mat4 modelRightEye_View = V * modelHead_World * Angel::Translate(headAnchor + rightOffset)*Angel::Scale(1.0f, 1.0f, zElongation);
        leftEye.draw(modelViewLoc, projectionLoc, modelLeftEye_View, projMatrix);
        rightEye.draw(modelViewLoc, projectionLoc, modelRightEye_View, projMatrix);

        vec3 leftOffset2  = vec3(-0.15f*scaleFactor, -0.15f*scaleFactor, 0.1f*scaleFactor);
        vec3 rightOffset2 = vec3(+0.15f*scaleFactor, -0.15f*scaleFactor, 0.1f*scaleFactor);
        mat4 modelLeftEye2_View = V * modelHead_World * Angel::Translate(headAnchor + leftOffset2) * Angel::Scale(0.7f, zElongation*0.7f, 0.7f);
        mat4 modelRightEye2_View = V * modelHead_World * Angel::Translate(headAnchor + rightOffset2) * Angel::Scale(0.7f, zElongation*0.7f, 0.7f);
        leftEye2.draw(modelViewLoc, projectionLoc, modelLeftEye2_View, projMatrix);
        rightEye2.draw(modelViewLoc, projectionLoc, modelRightEye2_View, projMatrix);

    std::vector<vec3> legAttachPoints = cephalothorax.getLegAttachmentPoints();

    // Leg swing is always calculated based on leg_animation_cycle_
    // which pauses if spider is not active
    float swing_phase_rad = leg_animation_cycle_ * 1.0f * static_cast<float>(M_PI);
    float current_swing_angle_deg = sin(swing_phase_rad) * max_leg_swing_angle_;

    float legRotationGroup1 = current_swing_angle_deg;
    float legRotationGroup2 = -current_swing_angle_deg;

    if (legAttachPoints.size() >= 8) {
        Leg* leg_objects[8] = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};
        float leg_anim_rotations[8] = {
            legRotationGroup1, legRotationGroup2, legRotationGroup2, legRotationGroup1,
            legRotationGroup1, legRotationGroup2, legRotationGroup2, legRotationGroup1
        };
        bool scale_x_negatively[8] = { true, false, true, false, true, false, true, false };

        mat4 legModelViewMatrix;

        for(int i=0; i<8; ++i) {
            mat4 leg_attachment_transform = Angel::Translate(legAttachPoints[i]);
            mat4 leg_animation_rotation_transform = Angel::RotateY(leg_anim_rotations[i]);
            mat4 leg_scale_transform = mat4();
            if (scale_x_negatively[i]) {
                leg_scale_transform = Angel::Scale(-1.0f, 1.0f, 1.0f);
            }
            mat4 leg_world_root_animated = spiderWorldTransform * leg_attachment_transform * leg_animation_rotation_transform * leg_scale_transform;
            legModelViewMatrix = V * leg_world_root_animated;
            leg_objects[i]->draw(modelViewLoc, projectionLoc,  legModelViewMatrix, projMatrix);
        }
    }
    }
} // namespace spider
// --- End of Spider.cpp ---
