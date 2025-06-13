
#version 330 core

in vec3 fNormal;
in vec3 fWorldPos;

out vec4 fragColor;

void main() {

    vec3 baseColor = vec3(0.9, 0.1, 0.1);


    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fNormal);
    float diff = max(dot(normal, lightDir), 0.6);


    vec3 finalColor = baseColor * diff + vec3(0.2, 0.0, 0.0) * pow(diff, 2.0);
    finalColor = min(finalColor, vec3(1.0));

    fragColor = vec4(finalColor, 1.0);
}