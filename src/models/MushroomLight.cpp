#include "MushroomLight.hpp"
#include "utils.hpp"
#include <glm/detail/func_geometric.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

void MushroomLight::initialize(bool isSkinned) { 
	ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath);
	scale = 30.0f * glm::vec3(1.0f, 1.0f, 1.0f);
};

void MushroomLight::update(float dt) { 
	ModelEntity::update(dt); 
};

std::string MushroomLight::modelDirectory = "../assets/MushroomLight/";
std::string MushroomLight::modelPath = modelDirectory + std::string("/scene.gltf");
std::string MushroomLight::vertexShaderPath = "../shaders/pbr.vert";
std::string MushroomLight::fragmentShaderPath = "../shaders/pbr.frag";