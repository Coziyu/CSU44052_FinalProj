#include "CheeseMoon.hpp"
#include <iostream>

// Static member definitions
SharedModelResources CheeseMoon::sharedResources(
    "../assets/cheese_moon/",
    "../assets/cheese_moon/scene.gltf",
    "../shaders/pbr.vert",
    "../shaders/pbr.frag"
);
bool CheeseMoon::resourcesInitialized = false;

void CheeseMoon::loadSharedResources() {
    if (resourcesInitialized) return;
    
    std::cout << "[CheeseMoon] Loading shared resources..." << std::endl;
    if (sharedResources.load(false)) {  // false = no skinning data
        resourcesInitialized = true;
        std::cout << "[CheeseMoon] Shared resources loaded successfully." << std::endl;
    } else {
        std::cerr << "[CheeseMoon] Failed to load shared resources!" << std::endl;
    }
}

void CheeseMoon::initializeInstance(bool skinned) {
    if (!resourcesInitialized) {
        loadSharedResources();
    }
    ModelEntity::initializeFromShared(&sharedResources, skinned);
}

void CheeseMoon::initialize(bool isSkinned) { 
    // Use shared resources path
    initializeInstance(isSkinned);
};

void CheeseMoon::update(float dt, glm::vec3 cameraPos) {
    // Rotate the front of the moon to always face the camera
    glm::vec3 direction = glm::normalize(cameraPos - position); // unit delta
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // TODO: Tune until it is the correct start
    glm::quat lookQuat = myQuatLookAt(direction, up);

	float baseRotationAngle = 3.14159f;
	glm::quat baseQuat = glm::angleAxis(baseRotationAngle, up);

    lookQuat =  lookQuat * baseQuat;

    rotationAxis = glm::axis(lookQuat);
    rotationAngle = glm::angle(lookQuat);

    ModelEntity::update(dt);
}