#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "../external/Angel/inlcude/Angel/Angel.h"

class Obstacle {
public:
    Obstacle(const vec3& pos, float size, GLuint shaderProgram);
    void draw(GLuint modelViewLoc, GLuint projectionLoc, const mat4& view, const mat4& projection);
    const vec3& getPosition() const;
    float getSize() const;

private:
    vec3 position_;
    float size_;
    GLuint shaderProgram_;
};

#endif // OBSTACLE_H