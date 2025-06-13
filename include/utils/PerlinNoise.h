// Reference: https://en.wikipedia.org/wiki/Perlin_noise
// Reference: *https://garagefarm.net/blog/perlin-noise-implementation-procedural-generation-and-simplex-noise


#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <vector>
#include <cmath>
#include <random>

class PerlinNoise {
public:
    PerlinNoise();
    float noise(float x, float y, float z);

private:
    std::vector<int> p;

    void generatePermutation();
    float fade(float t);
    float grad(int hash, float x, float y, float z);
    float lerp(float t, float a, float b);
};

#endif // PERLIN_NOISE_H