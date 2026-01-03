#ifndef LIGHTINGPARAMS_HPP
#define LIGHTINGPARAMS_HPP

#include <glm/glm.hpp>

// Central lighting parameters shared across all shaders
struct LightingParams {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

    LightingParams() 
        : lightPosition(-1326, 2200, 2571),
          lightIntensity(5e7f, 5e7f, 5e7f) {}
};

#endif // LIGHTINGPARAMS_HPP
