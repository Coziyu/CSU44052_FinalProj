#ifndef LIGHTINGPARAMS_HPP
#define LIGHTINGPARAMS_HPP

#include <glm/glm.hpp>

// Central lighting parameters shared across all shaders
struct LightingParams {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;
    glm::vec3 lightColor;
    
    // fade parameters for smooth object pop-in
    float fadeViewDistance;  // when objects start to fade out
    float fadeDistance;      // range where which fading occurs

    // PBR toggle options
    bool enableNormalMapping = true;
    bool enableGGXDistribution = true;
    bool enableGeometryTerm = true;
    bool enableFresnel = true;
    bool enableAmbientOcclusion = true;
    bool enableShadows = true;
    bool enableEmissive = true;
    bool enableToneMapping = true;

    LightingParams() 
        : lightPosition(-1326, 2200, 2571),
          lightIntensity(5e7f, 5e7f, 5e7f),
          lightColor(1.0f, 0.9f, 0.2f),
          fadeViewDistance(2500.0f),
          fadeDistance(500.0f),
          enableNormalMapping(true),
          enableGGXDistribution(true),
          enableGeometryTerm(true),
          enableFresnel(true),
          enableAmbientOcclusion(true),
          enableShadows(true),
          enableEmissive(true),
          enableToneMapping(true) {}
};

#endif // LIGHTINGPARAMS_HPP
