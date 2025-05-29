// Description: Source file for the Spider class, implementing its behavior and rendering.
#include "spider/Spider.h"
#include "global/GlobalConfig.h"
#include "spider/Leg.h"
#include <cmath> // For M_PI, sin, cos, fmod


namespace spider {

    Spider::Spider(GLuint shaderProgram)
    : cephalothorax(shaderProgram), abdomen(shaderProgram), head(shaderProgram),
      leftEye(shaderProgram), rightEye(shaderProgram), leftEye2(shaderProgram), rightEye2(shaderProgram),
      _shaderProgram(shaderProgram),
      leg(shaderProgram, 7, 0.6f, 1.3f),
      leg2(shaderProgram, 7, 0.6f, 1.3f),
      leg3(shaderProgram, 7, 0.6f, 1.5f),
      leg4(shaderProgram, 7, 0.6f, 1.5f),
      leg5(shaderProgram, 7, 0.6f, 1.3f),
      leg6(shaderProgram, 7, 0.6f, 1.3f),
      leg7(shaderProgram, 7, 0.6f, 1.5f),
      leg8(shaderProgram, 7, 0.6f, 1.5f),
      position(0.0f, 0.0f, 0.0f),
      current_forward_vector_(0.0f, 0.0f, 1.0f),
      is_walking_forward_(false),
      is_walking_backward_(false),
      is_turning_left_(false),
      is_turning_right_(false),
      walk_speed_(1.5f),
      turn_speed_(90.0f),
      current_yaw_angle_(0.0f),
      leg_animation_cycle_(0.0f),
      leg_animation_speed_(2.0f),
      max_leg_swing_angle_(35.0f),
      base_abdomen_tilt_angle_(ABDOMEN_TILT_ANGLE),
      abdomen_shake_cycle_(0.0f),
      abdomen_shake_speed_(3.0f),
      max_abdomen_shake_amplitude_(10.0f) {

        std::vector<float> angles = {20.0f, 15.0f, -10.0f, -35.0f, -20.0f, -30.0f, -10.0f};
        leg.setJointAngles(angles);
        leg2.setJointAngles(angles);
        leg3.setJointAngles(angles);
        leg4.setJointAngles(angles);
        leg5.setJointAngles(angles);
        leg6.setJointAngles(angles);
        leg7.setJointAngles(angles);
        leg8.setJointAngles(angles);

        calculateAndStoreInitialLegTipGroundContacts();
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

    bool is_active = is_walking_forward_ || is_walking_backward_ || is_turning_left_ || is_turning_right_;
    if (is_active) {
        leg_animation_cycle_ += leg_animation_speed_ * deltaTime;
        if (leg_animation_cycle_ > 1.0f) {
            leg_animation_cycle_ -= 1.0f;
        }

        abdomen_shake_cycle_ += abdomen_shake_speed_ * deltaTime;
        if (abdomen_shake_cycle_ > 1.0f) {
            abdomen_shake_cycle_ -= 1.0f;
        }
    }
}

void Spider::runIKForLeg(int legIndex, const vec3& worldTargetPosition) {
    if (legIndex < 0 || legIndex >= 8) return;

    mat4 currentSpiderWorldTransform = Angel::Translate(position) * Angel::RotateY(current_yaw_angle_);
    std::vector<vec3> legAttachPoints = cephalothorax.getLegAttachmentPoints();

    if (legAttachPoints.size() <= static_cast<size_t>(legIndex)) return;

    Leg* leg_objects[8] = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};

    bool scale_x_negatively[8] = { true, false, true, false, true, false, true, false };

    mat4 leg_attachment_transform = Angel::Translate(legAttachPoints[legIndex]);
    mat4 leg_scale_transform = mat4();
    if (scale_x_negatively[legIndex]) {
        leg_scale_transform = Angel::Scale(-1.0f, 1.0f, 1.0f);
    }

    mat4 leg_root_world_transform_for_ik = currentSpiderWorldTransform * leg_attachment_transform * leg_scale_transform;

    leg_objects[legIndex]->solveIKForTarget(worldTargetPosition, leg_root_world_transform_for_ik);
}


const std::vector<vec3>& Spider::getInitialLegTipGroundContacts() const {
    return initial_leg_tip_ground_contacts_;
}

void Spider::calculateAndStoreInitialLegTipGroundContacts() {
    initial_leg_tip_ground_contacts_.clear();
    initial_leg_tip_ground_contacts_.reserve(8);

    mat4 initial_R_yaw = Angel::RotateY(current_yaw_angle_);
    mat4 initial_T_translation = Angel::Translate(position);
    mat4 initial_spiderWorldTransform = initial_T_translation * initial_R_yaw;

    std::vector<vec3> legAttachPoints = cephalothorax.getLegAttachmentPoints();
    if (legAttachPoints.size() < 8) {
        return;
    }

    Leg* leg_objects[8] = {&leg, &leg2, &leg3, &leg4, &leg5, &leg6, &leg7, &leg8};

    float initial_leg_anim_rotations[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bool scale_x_negatively[8] = { true, false, true, false, true, false, true, false };

    for (int i = 0; i < 8; ++i) {
        mat4 leg_attachment_transform = Angel::Translate(legAttachPoints[i]);
        mat4 leg_rotation_transform = Angel::RotateY(initial_leg_anim_rotations[i]);
        mat4 leg_scale_transform = mat4();
        if (scale_x_negatively[i]) {
            leg_scale_transform = Angel::Scale(-1.0f, 1.0f, 1.0f);
        }
        mat4 legRootWorldTransform = initial_spiderWorldTransform * leg_attachment_transform * leg_rotation_transform * leg_scale_transform;
        std::vector<vec3> segment_world_ends = leg_objects[i]->getAllJointAndTipWorldPositions(legRootWorldTransform);

        if (!segment_world_ends.empty()) {
            vec3 leg_tip_world = segment_world_ends.back();
            vec3 ground_contact = vec3(leg_tip_world.x, 0.0f, leg_tip_world.z);
            initial_leg_tip_ground_contacts_.push_back(ground_contact);
        } else {
            vec4 attach_point_world_v4 = initial_spiderWorldTransform * vec4(legAttachPoints[i], 1.0f);
            initial_leg_tip_ground_contacts_.emplace_back(attach_point_world_v4.x, 0.0f, attach_point_world_v4.z);
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
    float swing_phase_rad = leg_animation_cycle_ * 2.0f * static_cast<float>(M_PI);
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