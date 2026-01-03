#include "Phoenix.hpp"
#include "utils.hpp"
#include <glm/detail/func_geometric.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

void Phoenix::initialize(bool isSkinned) { 
	ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath);
	animationSpeed = 0.5;
};

void Phoenix::update(float dt) { 
	// Fly in a circle of radius 1000, centered at (500,1500,500);
	// At 0 secs the bird is at (500,1500,500)
	// At 10 secs, the bird is at (500,1500,500)
	float radius = 1500.0f;
	glm::vec3 center = glm::vec3(500.0f, 1000.0f, 500.0f);
	float angle = modelTime * 0.5;
	float dangle = angle + dt;
	position = center + glm::vec3(radius * cos(angle), 400.0f * cos(angle / 5), radius * sin(angle));
	glm::vec3 positiond = center + glm::vec3(radius * cos(dangle), 400.0f * cos(dangle / 5), radius * sin(dangle));

	glm::vec3 direction = glm::normalize(positiond - position); // unit delta
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // TODO: Tune until it is the correct start

	glm::quat lookQuat = myQuatLookAt(direction, up);

	// Phoenix should rotate such that it is tangential to the circle
	float baseRotationAngle = 3.14159f / 2.0f;
	glm::quat baseQuat = glm::angleAxis(baseRotationAngle, up);

	lookQuat =  lookQuat * baseQuat;

	rotationAxis = glm::axis(lookQuat);
	rotationAngle = glm::angle(lookQuat);
	

	ModelEntity::update(dt); 
};

std::string Phoenix::modelDirectory = "../assets/phoenix_bird/";
std::string Phoenix::modelPath = modelDirectory + std::string("/scene.gltf");
std::string Phoenix::vertexShaderPath = "../shaders/phoenix.vert";
std::string Phoenix::fragmentShaderPath = "../shaders/phoenix.frag";