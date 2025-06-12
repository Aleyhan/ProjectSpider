#version 330 core

 in vec3 fNormal;
 in vec3 fWorldPos;

 out vec4 fragColor;

 void main() {

     vec3 baseColor = vec3(0.2, 0.2, 0.2);


     vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
     vec3 normal = normalize(fNormal);
     float diff = max(dot(normal, lightDir), 0.4);

     vec3 finalColor = baseColor * diff;
     fragColor = vec4(finalColor, 1.0);
 }