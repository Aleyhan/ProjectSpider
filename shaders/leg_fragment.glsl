#version 330 core

    in vec3 fNormal;
    in vec3 fWorldPos;

    out vec4 fragColor;

    void main() {
        // Gradient için kahverengi tonları
        vec3 darkBrown = vec3(0.4, 0.2, 0.05);   // Koyu kahverengi (biraz daha zengin)
        vec3 lightBrown = vec3(0.7, 0.4, 0.1);  // Açık kahverengi (biraz daha zengin)

        // fWorldPos.y değerini 0-1 arasına normalize et (veya istediğiniz bir eksene göre ayarlayın)
        // Y eksenindeki pozisyona göre bir gradient oluşturuyoruz.
        // Pozitif Y daha açık, negatif Y daha koyu olacak şekilde ayarlanabilir.
        // Bu örnekte, -0.5 ile 0.5 arasındaki Y değerlerini 0-1 arasına map ediyoruz.
        float gradientFactor = clamp((fWorldPos.y + 0.5), 0.0, 1.0);

        // İki renk arasında interpolasyon
        vec3 baseColor = mix(darkBrown, lightBrown, gradientFactor);

        // Gelişmiş aydınlatma
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); // Işık yönü
        vec3 normal = normalize(fNormal); // Normal vektörü normalize et

        // Ambient light (ortam ışığı)
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * baseColor;

        // Diffuse light (dağınık ışık)
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * baseColor;

        // Final renk (ambient ve diffuse ışığın toplamı)
        vec3 finalColor = ambient + diffuse;

        // Kenarları biraz daha koyu yap (isteğe bağlı stilistik etki)
        // Kamera yönüne dik normaller daha koyu görünebilir.
        // Bu örnekte viewDir (kamera yönü) olmadığı için basitleştirilmiş bir kenar efekti.
        // Örneğin, Z eksenine dik yüzeyler için bir faktör.
        float edgeFactor = 1.0 - pow(abs(dot(normal, normalize(vec3(0.0, 0.0, 1.0)))), 4.0); // Daha yumuşak geçiş için üs artırıldı
        finalColor = mix(finalColor, finalColor * 0.6, edgeFactor); // Kenarları %60 oranında koyulaştır

        // Renklerin 1.0'ı geçmemesini sağla (clamping)
        finalColor = clamp(finalColor, 0.0, 1.0);

        fragColor = vec4(finalColor, 1.0);
    }