#ifndef LIGHTINGPARAMS_HPP
#define LIGHTINGPARAMS_HPP

#include <glm/glm.hpp>

// Central lighting parameters shared across all shaders
struct LightingParams {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

    LightingParams() 
        : lightPosition(-275.0f, 500.0f, 800.0f),
          lightIntensity(5e6f, 5e6f, 5e6f) {}
};

#endif // LIGHTINGPARAMS_HPP
