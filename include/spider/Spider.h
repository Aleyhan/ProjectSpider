// Description: Header file for the Spider class, defining its structure and functionalities.
#ifndef SPIDER_H
#define SPIDER_H

#include <MacTypes.h>

#include "Cephalothorax.h"
#include "Abdomen.h"
#include "Head.h"
#include "Eye.h"
#include "Leg.h"
#include <vector>
#include <string>

namespace spider {

    class Spider {
    public:
        Spider(GLuint cephalothoraxShader, GLuint abdomenShader, GLuint legShader, GLuint eyeShader);
        void setPosition(const vec3& pos);
        const vec3& getPosition() const;

        void startWalkingForward();
        void stopWalkingForward();
        void startWalkingBackward();
        void stopWalkingBackward();
        void startTurningLeft();
        void stopTurningLeft();
        void startTurningRight();
        void stopTurningRight();

        void update(float deltaTime);

        void applyIKToAllLegs(std::vector<Leg*>& legs,
                                      const std::vector<Angel::vec3>& targets,
                                      float param1, int param2, int param3,
                                      float param4,
                                      const std::vector<std::vector<float>>& matrix1,
                                      const std::vector<std::vector<float>>& matrix2);

        void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& viewMatrix, const mat4& projMatrix);

        const std::vector<vec3>& getInitialLegTipGroundContacts() const;
        Leg leg, leg2, leg3, leg4, leg5, leg6, leg7, leg8;
        Cephalothorax cephalothorax;

        bool jumpTriggered; // Global flag to trigger jump

        std::vector<std::pair<float, float>> getXYLengthsForAllAttachments(const std::vector<vec3>& attachPoints);

        void moveBodyUp();
        void moveBodyDown();

        void jump(float deltaTime, float jumpDuration);

    private:
        void calculateAndStoreInitialLegTipGroundContacts();


        Abdomen abdomen;
        Head head;
        Eye leftEye, rightEye, leftEye2, rightEye2;

        GLuint _shaderProgram;
        vec3 position;
        vec3 current_forward_vector_;

        bool is_walking_forward_;
        bool is_walking_backward_;
        bool is_turning_left_;
        bool is_turning_right_;

        float walk_speed_;
        float turn_speed_;
        float current_yaw_angle_;
    float scale_ = 1.0f;

        float leg_animation_cycle_;
        float leg_animation_speed_;
        float max_leg_swing_angle_;

        float base_abdomen_tilt_angle_;
        float abdomen_shake_cycle_;
        float abdomen_shake_speed_;
        float max_abdomen_shake_amplitude_;

    };

} // namespace spider

#endif // SPIDER_H
// --- End of Spider.h ---
