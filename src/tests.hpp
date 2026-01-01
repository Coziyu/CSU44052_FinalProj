#ifndef UTILS_HPP
#define UTILS_HPP

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "Perlin.hpp"

void testRenderPerlinNoise() {
  PerlinNoise pn(5);

  // Generate a 512x512 texture
  const int size = 512;
  unsigned char *data = new unsigned char[size * size];
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      const int scale = 2;
      double nx = scale * static_cast<double>(x) / static_cast<double>(size);
      double ny = scale * static_cast<double>(y) / static_cast<double>(size);
      double noiseValue = pn.octavePerlin(nx, ny, 3, 0.6);
      data[y * size + x] = static_cast<unsigned char>(noiseValue * 255);
    }
  }

  stbi_write_png("../perlin_texture.png", size, size, 1, data, size);
  delete[] data;
}

#endif // UTILS_HPP
