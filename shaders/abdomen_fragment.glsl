#version 330 core

in vec3 fNormal;
in vec3 fWorldPos;
in vec3 fColor;

out vec4 fragColor;

void main() {
    // OPTION 1: Use world position as debug color
    vec3 color = fWorldPos * 0.5 + 0.5;

    /*
    // OPTION 2: Use normal-based shading
    vec3 lightDir  = normalize(vec3(0.0, 0.0, 1.0));
    float diff     = max(dot(normalize(fNormal), lightDir), 0.0);
    vec3 baseColor = fColor;  // fColor from vAux (either normal or actual color)
    vec3 color     = diff * baseColor;
    */
    fragColor = vec4(color, 1.0);
}
