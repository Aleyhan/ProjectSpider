#version 330 core

in vec3 fNormal;
in vec3 fWorldPos;

out vec4 fragColor;

void main() {
    // Gradient için koyu kahverengiden açık kahverengiye geçiş
    vec3 darkBrown = vec3(0.35, 0.15, 0.05);   // Koyu kahverengi
    vec3 lightBrown = vec3(0.65, 0.35, 0.15);  // Açık kahverengi

    // fWorldPos.y değerini 0-1 arasına normalize et
    float gradient = clamp(fWorldPos.y + 0.5, 0.0, 1.0);

    // İki renk arasında interpolasyon
    vec3 baseColor = mix(darkBrown, lightBrown, gradient);

    // Gelişmiş aydınlatma
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fNormal);

    // Ambient light
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * baseColor;

    // Diffuse light
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * baseColor;

    // Final renk
    vec3 finalColor = ambient + diffuse;

    // Kenarları biraz daha koyu yap
    float edgeFactor = 1.0 - pow(abs(dot(normal, vec3(0.0, 0.0, 1.0))), 2.0);
    finalColor = mix(finalColor, finalColor * 0.7, edgeFactor);

    fragColor = vec4(finalColor, 1.0);
}