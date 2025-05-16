#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vAux; // Either vNormal (object) or vColor (axes)

uniform mat4 model_view;
uniform mat4 projection;

out vec3 fNormal;
out vec3 fColor;
out vec3 fWorldPos;

void main() {
    gl_Position = projection * model_view * vec4(vPosition, 1.0);

    // Determine world position (used for debug coloring)
    fWorldPos = vPosition;

    // If using normals (for shaded object), transform it
    fNormal = mat3(model_view) * vAux;

    // Pass vAux directly as color for objects like axes
    fColor = vAux;
}
