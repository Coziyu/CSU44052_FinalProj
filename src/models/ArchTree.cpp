#include "ArchTree.hpp"
#include "ModelEntity.hpp"
#include <iostream>

// Static member definitions
SharedModelResources ArchTree::sharedResources(
    "../assets/arch_tree/",
    "../assets/arch_tree/scene.gltf",
    "../shaders/pbr.vert",
    "../shaders/pbr.frag"
);
bool ArchTree::resourcesInitialized = false;

void ArchTree::loadSharedResources() {
    if (resourcesInitialized) return;
    // TODO: Better shared logging system??
    std::cout << "[ArchTree] Loading shared resources..." << std::endl;
    if (sharedResources.load(true)) {  // true = prepare skinning data
        resourcesInitialized = true;
        std::cout << "[ArchTree] Shared resources loaded successfully." << std::endl;
    } else {
        std::cerr << "[ArchTree] Failed to load shared resources!" << std::endl;
    }
}

void ArchTree::initializeInstance(bool skinned) {
    if (!resourcesInitialized) {
        loadSharedResources();
    }
    ModelEntity::initializeFromShared(&sharedResources, skinned);
}

void ArchTree::initialize(bool isSkinned) {
    // Use shared resources path
    initializeInstance(isSkinned);
}

void ArchTree::update(float dt) {
    static float initialY = position.y;
    // Bob up and down slowly
    float bobHeight = 50.0f;
    float bobSpeed = 0.5f;
    position.y = initialY + bobHeight * cos(modelTime * bobSpeed);

    ModelEntity::update(dt);
}
