#include "spider/Spider.h"
#include "global/GlobalConfig.h"
#include "spider/Leg.h"

namespace spider {

    // Update the Spider constructor to create a better test leg
    Spider::Spider(GLuint shaderProgram)
    : cephalothorax(shaderProgram), abdomen(shaderProgram), head(shaderProgram),
      leftEye(shaderProgram), rightEye(shaderProgram), leftEye2(shaderProgram), rightEye2(shaderProgram),
      _shaderProgram(shaderProgram),
      leg(shaderProgram, 7, 0.6f, 1.3f),
      leg2(shaderProgram, 7, 0.6f, 1.3f),
      leg3(shaderProgram, 7, 0.6f, 1.5f),
      leg4(shaderProgram, 7, 0.6f, 1.5f){  // Initialize second leg// Initialize in constructor

        // Initialize joint angles for the leg
        std::vector<float> angles = {20.0f, 15.0f, -10.0f, -35.0f, -20.0f, -30.0f, -10.0f};

        leg.setJointAngles(angles);
        leg2.setJointAngles(angles);  // Same angles for second leg
        leg3.setJointAngles(angles);  // Same angles for third leg
        leg4.setJointAngles(angles);  // Same angles for fourth leg

    }


    void Spider::setPosition(const vec3& pos) {
        position = pos;

    }

    const vec3& Spider::getPosition() const {
        return position;
    }



        void Spider::draw(
            GLuint modelViewLoc,
            GLuint projectionLoc,
            const mat4& viewMatrix,
            const mat4& projMatrix
        ) {
        // -------------------------------
        // Common transforms
        // -------------------------------
        mat4 T_root = Translate(position);         // Spider position in world
        mat4 V = viewMatrix;

        // -------------------------------
        // 1. Draw Cephalothorax
        // -------------------------------
        mat4 modelCT = V * T_root;
        cephalothorax.draw(modelViewLoc, projectionLoc, modelCT, projMatrix);

        // -------------------------------
        // 2. Draw Abdomen attached behind
        // -------------------------------
        const float rz = ABDOMEN_RADIUS * ABDOMEN_SCALE_Z;

        mat4 R_tilt    = RotateX(ABDOMEN_TILT_ANGLE);   // tilt around X
        mat4 T_pivot   = Translate(0, 0, -rz*1.8f);     // rotate around back edge

        mat4 abdomenLocal = R_tilt * T_pivot;
        mat4 modelAbdomen = V * T_root * abdomenLocal;

        abdomen.draw(modelViewLoc, projectionLoc, modelAbdomen, projMatrix);

        // --- 3. Draw Head ---
        vec3 localHeadPos = cephalothorax.getHeadAnchorPoint(); // local coord
        // Calculate 10% of abdomen radius for Z offset
        float zOffset = -0.1f * ABDOMEN_RADIUS;

        mat4 modelHead = viewMatrix
            * Translate(position)       // world spider pos
            * Translate(localHeadPos)
            * Translate(0, 0, 0); // Move 10% back in Z direction;  // head relative to cephalothorax

        head.draw(modelViewLoc, projectionLoc, modelHead, projMatrix);

        // Get the most front vertex in local head space
        vec3 headAnchor = head.getMostFrontVertex();
        float scaleFactor = ABDOMEN_RADIUS*HEAD_SCALE / (DEFAULT_ABDOMEN_RADIUS*0.5); // Add DEFAULT_ABDOMEN_RADIUS constant

        // Small local offsets for eye placement
        vec3 leftOffset  = vec3(-0.07f*scaleFactor, +0.05f*scaleFactor, 0.1f*scaleFactor);
        vec3 rightOffset = vec3(+0.07f*scaleFactor, +0.05f*scaleFactor, 0.1f*scaleFactor);
        float zElongation = 1.2f; // Adjust this value to control elongation amount

        // Create transforms that place eyes at the front vertex plus offsets
        mat4 modelLeftEye  = modelHead * Translate(headAnchor + leftOffset)*Scale(1.0f, 1.0f, zElongation);
        mat4 modelRightEye = modelHead * Translate(headAnchor + rightOffset)*Scale(1.0f, 1.0f, zElongation);

        leftEye.draw(modelViewLoc, projectionLoc, modelLeftEye, projMatrix);
        rightEye.draw(modelViewLoc, projectionLoc, modelRightEye, projMatrix);

        // Small local offsets for eye placement
        vec3 leftOffset2  = vec3(-0.15f*scaleFactor, -0.15f*scaleFactor, 0.1f*scaleFactor);
        vec3 rightOffset2 = vec3(+0.15f*scaleFactor, -0.15f*scaleFactor, 0.1f*scaleFactor);


        // Create transforms with z-elongation for second pair of eyes
        mat4 modelLeftEye2 = modelHead * Translate(headAnchor + leftOffset2) * Scale(0.7f, zElongation*0.7f, 0.7f);
        mat4 modelRightEye2 = modelHead * Translate(headAnchor + rightOffset2) * Scale(0.7f, zElongation*0.7f, 0.7f);

        leftEye2.draw(modelViewLoc, projectionLoc, modelLeftEye2, projMatrix);
        rightEye2.draw(modelViewLoc, projectionLoc, modelRightEye2, projMatrix);




        // Get leg attachment points from cephalothorax
        std::vector<vec3> legAttachPoints = cephalothorax.getLegAttachmentPoints();

        // Animate the rotation
        static float time = 0.0f;
        time += 0.15f;
        legRotation = 5.0f * sin(time * 0.5f);
        legRotation2 = 5.0f * sin(time * 0.5f*-1);



        // Draw the leg
        mat4 legModelMatrix = Translate(legAttachPoints[1]) * RotateY(legRotation);
        leg.draw(modelViewLoc, projectionLoc, V * legModelMatrix, projMatrix);

        // Draw the second leg at attachment point 3 with reversed X direction
        mat4 leg2ModelMatrix = Translate(legAttachPoints[0]) * RotateY(-1* legRotation2) * Scale(-1.0f, 1.0f, 1.0f);
        leg2.draw(modelViewLoc, projectionLoc, V * leg2ModelMatrix, projMatrix);

        // Draw the second leg at attachment point 3 with reversed X direction
        mat4 leg3ModelMatrix = Translate(legAttachPoints[2]) * RotateY(-1* legRotation) * Scale(-1.0f, 1.0f, 1.0f);
        leg3.draw(modelViewLoc, projectionLoc, V * leg3ModelMatrix, projMatrix);

        // Draw the leg
        mat4 leg4ModelMatrix = Translate(legAttachPoints[3]) * RotateY(legRotation2);
        leg4.draw(modelViewLoc, projectionLoc, V * leg4ModelMatrix, projMatrix);
        // Dynamically adjust leg4's joint angles based on time
        std::vector<float> dynamicAngles = {
            20.0f + 10.0f * sin(time * 0.3f),              // First joint
            15.0f + 8.0f * sin(time * 0.4f + 0.5f),        // Second joint
            -10.0f + 15.0f * sin(time * 0.35f + 1.0f),     // Third joint
            -35.0f + 12.0f * sin(time * 0.45f + 1.5f),     // Fourth joint
            -20.0f + 10.0f * sin(time * 0.5f + 2.0f),      // Fifth joint
            -30.0f + 8.0f * sin(time * 0.3f + 2.5f),       // Sixth joint
            -10.0f + 5.0f * sin(time * 0.25f + 3.0f)       // Seventh joint
        };

        // Apply the dynamic angles to leg4
        leg4.setJointAngles(dynamicAngles);
    }


}

