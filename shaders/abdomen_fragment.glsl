
#version 330 core

in vec3 fNormal;
in vec3 fWorldPos;

out vec4 fragColor;

void main() {
    vec3 baseColor;

    // Spider body parts coloring - daha açık renkler
    if (fWorldPos.y > 0.05) {
        // Body - Koyu gri (siyah yerine)
        baseColor = vec3(0.3, 0.3, 0.3);
    } else if (length(fWorldPos) > 0.8) {
        // Legs - Daha açık kahverengi
        baseColor = vec3(0.75, 0.35, 0.1);
    } else if (fWorldPos.z > 0.9) {
        // Eyes - Parlak kırmızı
        baseColor = vec3(1.0, 0.2, 0.2);
    } else {
        // Default - Orta gri
        baseColor = vec3(0.4, 0.4, 0.4);
    }

    // Geliştirilmiş ışıklandırma
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fNormal);
    float diff = max(dot(normal, lightDir), 0.5); // Ambient ışığı 0.5'e çıkardık

    // Ekstra parlaklık için hafif bir artış
    vec3 finalColor = baseColor * diff * 1.2; // %20 daha parlak

    // Renklerin aşırı parlak olmasını önle
    finalColor = min(finalColor, vec3(1.0));

    fragColor = vec4(finalColor, 1.0);
}