#include "MushroomLight.hpp"
#include <iostream>

// Static member definitions
SharedModelResources MushroomLight::sharedResources(
    "../assets/MushroomLight/",
    "../assets/MushroomLight/scene.gltf",
    "../shaders/pbr.vert",
    "../shaders/pbr.frag"
);
bool MushroomLight::resourcesInitialized = false;

void MushroomLight::loadSharedResources() {
    if (resourcesInitialized) return;
    
    std::cout << "[MushroomLight] Loading shared resources..." << std::endl;
    if (sharedResources.load(false)) {  // false = no skinning data
        resourcesInitialized = true;
        std::cout << "[MushroomLight] Shared resources loaded successfully." << std::endl;
    } else {
        std::cerr << "[MushroomLight] Failed to load shared resources!" << std::endl;
    }
}

void MushroomLight::initializeInstance(bool skinned) {
    if (!resourcesInitialized) {
        loadSharedResources();
    }
    ModelEntity::initializeFromShared(&sharedResources, skinned);
    // Set default mushroom scale
    scale = 30.0f * glm::vec3(1.0f, 1.0f, 1.0f);
}

void MushroomLight::initialize(bool isSkinned) { 
    // Use shared resources path
    initializeInstance(isSkinned);
}

void MushroomLight::update(float dt) { 
    // No animation for mushrooms
    // (The shared model has no animations)
}
