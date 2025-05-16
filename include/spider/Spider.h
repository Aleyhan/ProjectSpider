#pragma once
#include <vector>
#include <GL/glew.h>
#include "../external/Angel/inlcude/Angel/Angel.h"
#include "spider/Abdomen.h"
#include "spider/Cephalothorax.h"
#include "spider/Head.h"
#include "spider/Eye.h"
#include "spider/Leg.h"          // ⬅️ NEW
#include "global/GlobalConfig.h" // ⬅️ NEW

namespace spider {

    class Spider {
    public:
        explicit Spider(GLuint shaderProgram);

        // ⬇️ remove the final “const” so we can mutate leg poses inside draw
        void draw(GLuint modelViewLoc,
                  GLuint projectionLoc,
                  const mat4& viewMatrix,
                  const mat4& projMatrix);

        void        setPosition(const vec3& pos);
        const vec3& getPosition() const;
        float legRotation = -45.0f;
        float legRotation2 = -45.0f;



    private:

        GLuint _shaderProgram;

        vec3 position = vec3(BODY_START_X, BODY_START_Y, BODY_START_Z);

        Cephalothorax cephalothorax;
        Abdomen       abdomen;
        Head          head;

        Eye leftEye, rightEye, leftEye2, rightEye2;
        spider::Leg leg;  // Add this member variable
        spider::Leg leg2;  // Add this new leg
        spider::Leg leg3;  // Add this new leg
        spider::Leg leg4;  // Add this new leg

    };

    void testLegSegmentDrawing(GLuint luint);
} // namespace spider
