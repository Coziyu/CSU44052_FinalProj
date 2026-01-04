#ifndef LIGHTINGPARAMS_HPP
#define LIGHTINGPARAMS_HPP

#include <glm/glm.hpp>

// Central lighting parameters shared across all shaders
struct LightingParams {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;
    
    // fade parameters for smooth object pop-in
    float fadeViewDistance;  // when objects start to fade out
    float fadeDistance;      // range where which fading occurs

    LightingParams() 
        : lightPosition(-1326, 2200, 2571),
          lightIntensity(5e7f, 5e7f, 5e7f),
          fadeViewDistance(2500.0f),
          fadeDistance(500.0f) {}
};

#endif // LIGHTINGPARAMS_HPP
