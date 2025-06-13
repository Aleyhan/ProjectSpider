
#version 330 core

in vec3 fNormal;
in vec3 fWorldPos;

out vec4 fragColor;

void main() {
    vec3 baseColor;

    if (fWorldPos.y > 0.05) {
        baseColor = vec3(0.3, 0.3, 0.3);
    } else if (length(fWorldPos) > 0.8) {
        baseColor = vec3(0.75, 0.35, 0.1);
    } else if (fWorldPos.z > 0.9) {
        baseColor = vec3(1.0, 0.2, 0.2);
    } else {
        baseColor = vec3(0.4, 0.4, 0.4);
    }


    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fNormal);
    float diff = max(dot(normal, lightDir), 0.5);


    vec3 finalColor = baseColor * diff * 1.2;


    finalColor = min(finalColor, vec3(1.0));

    fragColor = vec4(finalColor, 1.0);
}