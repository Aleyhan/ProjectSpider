#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec2 vTexCoord;

out vec2 texCoord;

uniform mat4 model_view;
uniform mat4 projection;

void main() {
    texCoord = vTexCoord;
    gl_Position = projection * model_view * vec4(vPosition, 1.0);
}