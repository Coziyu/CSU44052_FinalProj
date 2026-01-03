#include "CheeseMoon.hpp"

void CheeseMoon::initialize(bool isSkinned) { ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath); };

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

std::string CheeseMoon::modelDirectory = "../assets/cheese_moon/";
std::string CheeseMoon::modelPath = modelDirectory + std::string("/scene.gltf");
std::string CheeseMoon::vertexShaderPath = "../shaders/pbr.vert";
std::string CheeseMoon::fragmentShaderPath = "../shaders/pbr.frag";