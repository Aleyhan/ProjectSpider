// PerlinNoise.cpp
// Reference: *https://garagefarm.net/blog/perlin-noise-implementation-procedural-generation-and-simplex-noise
// Reference: https://en.wikipedia.org/wiki/Perlin_noise
// PerlinNoise.cpp is written by AI with GEMINI and ChatGPT

#include "utils/PerlinNoise.h"


PerlinNoise::PerlinNoise() {
    generatePermutation();
}

void PerlinNoise::generatePermutation() {
    std::vector<int> temp(256);
    std::iota(temp.begin(), temp.end(), 0);
    std::shuffle(temp.begin(), temp.end(), std::default_random_engine(0)); // Fixed seed
    p.resize(512);
    for (int i = 0; i < 256; ++i) {
        p[i] = p[i + 256] = temp[i];
    }
}

float PerlinNoise::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float PerlinNoise::lerp(float t, float a, float b) {
    return a + t * (b - a);
}

// In PerlinNoise.cpp

float PerlinNoise::noise(float x, float y, float z) {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z); // Calculate w for z

    // Hash coordinates of the 8 cube corners
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    // Add blended results from 8 corners of the cube
    float res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),      // AND ADD THE GRADIENT VALUE FOR THE PRIMARY AXIS
                                         grad(p[BA], x - 1, y, z)),   // AND ADD THE GRADIENT VALUE FOR THE NEXT AXIS
                                lerp(u, grad(p[AB], x, y - 1, z),    // AND SO ON
                                         grad(p[BB], x - 1, y - 1, z))),
                        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                         grad(p[BA + 1], x - 1, y, z - 1)),
                                lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                         grad(p[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0f) / 2.0f; // Return value in range [0, 1]
}