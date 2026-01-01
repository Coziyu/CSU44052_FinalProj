#include "Perlin.hpp"


PerlinNoise::PerlinNoise(int repeat) : repeats(repeat) {};
double PerlinNoise::fade(double t) {
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::grad2D(int hash, double x, double y) {
    // Bit mask for 2d
    switch (hash & 3) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        default: return 0;
    }
}

double PerlinNoise::perlin2D(double x, double y) const {
    if (repeats > 0) {
        x = x - repeats * floor(x / repeats);
        y = y - repeats * floor(y / repeats);
    }

    // Find unit cell coordinates
    int xi = static_cast<int>(floor(x)) & 255;
    int yi = static_cast<int>(floor(y)) & 255;
    double xf = glm::fract(x);
    double yf = glm::fract(y);
    
    // LI weights
    double u = fade(xf);
    double v = fade(yf);

    int aa = p[p[xi] + yi];
    int ab = p[p[xi] + inc(yi)];
    int ba = p[p[inc(xi)] + yi];
    int bb = p[p[inc(xi)] + inc(yi)];

    double x1, x2;
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
// glm::vec3 PerlinNoise::perlin2DDerivatives(double x, double y) const {
//     if (repeats > 0) {
//         x = x - repeats * floor(x / repeats);
//         y = y - repeats * floor(y / repeats);
//     }

//     // Find unit cell coordinates
//     int xi = static_cast<int>(floor(x)) & 255;
//     int yi = static_cast<int>(floor(y)) & 255;
//     double xf = glm::fract(x);
//     double yf = glm::fract(y);
    
//     // LI weights
//     double u = fade(xf);
//     double v = fade(yf);

//     int aa = p[p[xi] + yi];
//     int ab = p[p[xi] + inc(yi)];
//     int ba = p[p[inc(xi)] + yi];
//     int bb = p[p[inc(xi)] + inc(yi)];

//     double x1, x2;
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
 * @return double 
 */
double PerlinNoise::octavePerlin(double x, double y, int octaves, double persistence, double lacunarity) const {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;  // Used for normalizing result to [0,1]

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
