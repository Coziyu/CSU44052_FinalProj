#include "Perlin.hpp"
#include <omp.h>

PerlinNoise::PerlinNoise(int repeat) : repeats(repeat) {};
float PerlinNoise::fade(float t) {
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::grad2D(int hash, float x, float y) {
    // Bit mask for 2d
    switch (hash & 3) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        default: return 0;
    }
}

float PerlinNoise::perlin2D(float x, float y) const {
    if (repeats > 0) {
        x = x - repeats * floor(x / repeats); 
        y = y - repeats * floor(y / repeats);
    }

    // Find unit cell coordinates
    int xi = static_cast<int>(floor(x)) & 255;
    int yi = static_cast<int>(floor(y)) & 255;
    float xf = glm::fract(x);
    float yf = glm::fract(y);
    
    // LI weights
    float u = fade(xf);
    float v = fade(yf);

    int aa = p[p[xi] + yi];
    int ab = p[p[xi] + inc(yi)];
    int ba = p[p[inc(xi)] + yi];
    int bb = p[p[inc(xi)] + inc(yi)];

    float x1, x2;
    x1 = glm::mix(
        grad2D(aa, xf, yf),
        grad2D(ba, xf - 1, yf),
        u
    );
    x2 = glm::mix(
        grad2D(ab, xf, yf - 1),
        grad2D(bb, xf - 1, yf - 1),
        u
    );
    return (glm::mix(x1, x2, v) + 1) / 2; // Normalize to [0,1]
}

// TODO: If time allows, implement derivative trick in:
// https://iquilezles.org/articles/morenoise/
// https://www.youtube.com/watch?v=gsJHzBTPG0Y
// /**
//  * @brief Returns the value and the derivatives of the 2D Perlin noise at (x, y)
//  * @return glm::vec3 
//  */
// glm::vec3 PerlinNoise::perlin2DDerivatives(float x, float y) const {
//     if (repeats > 0) {
//         x = x - repeats * floor(x / repeats);
//         y = y - repeats * floor(y / repeats);
//     }

//     // Find unit cell coordinates
//     int xi = static_cast<int>(floor(x)) & 255;
//     int yi = static_cast<int>(floor(y)) & 255;
//     float xf = glm::fract(x);
//     float yf = glm::fract(y);
    
//     // LI weights
//     float u = fade(xf);
//     float v = fade(yf);

//     int aa = p[p[xi] + yi];
//     int ab = p[p[xi] + inc(yi)];
//     int ba = p[p[inc(xi)] + yi];
//     int bb = p[p[inc(xi)] + inc(yi)];

//     float x1, x2;
//     x1 = glm::mix(
//         grad2D(aa, xf, yf),
//         grad2D(ba, xf - 1, yf),
//         u
//     );
//     x2 = glm::mix(
//         grad2D(ab, xf, yf - 1),
//         grad2D(bb, xf - 1, yf - 1),
//         u
//     );
//     return (glm::mix(x1, x2, v) + 1) / 2; // Normalize to [0,1]
// }

/**
 * @brief noise with multiple samples layered together
 * 
 * @param octaves number of frequency layers
 * @param persistence value between 0 and 1, controlling subsequent amplitude falloff
 * @return float 
 */
float PerlinNoise::octavePerlin(float x, float y, int octaves, float persistence, float lacunarity) const {
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0;  // Used for normalizing result to [0,1]
    
    for (int i = 0; i < octaves; i++) {
        total += perlin2D(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

// For handling repeated noise patterns
int PerlinNoise::inc(int num) const {
    num++;
    if (repeats > 0) num = num % repeats;
    return num;
}
