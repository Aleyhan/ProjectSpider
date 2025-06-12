#version 330 core

    in vec3 fNormal;
    in vec3 fWorldPos;

    out vec4 fragColor;

    void main() {

        vec3 darkBrown = vec3(0.4, 0.2, 0.05);
        vec3 lightBrown = vec3(0.7, 0.4, 0.1);





        float gradientFactor = clamp((fWorldPos.y + 0.5), 0.0, 1.0);


        vec3 baseColor = mix(darkBrown, lightBrown, gradientFactor);


        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
        vec3 normal = normalize(fNormal);


        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * baseColor;


        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * baseColor;


        vec3 finalColor = ambient + diffuse;





        float edgeFactor = 1.0 - pow(abs(dot(normal, normalize(vec3(0.0, 0.0, 1.0)))), 4.0);
        finalColor = mix(finalColor, finalColor * 0.6, edgeFactor);


        finalColor = clamp(finalColor, 0.0, 1.0);

        fragColor = vec4(finalColor, 1.0);
    }