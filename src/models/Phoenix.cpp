#include "Phoenix.hpp"

void Phoenix::initialize(bool isSkinned) { ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath); };

std::string Phoenix::modelDirectory = "../assets/phoenix_bird/";
std::string Phoenix::modelPath = modelDirectory + std::string("/scene.gltf");
std::string Phoenix::vertexShaderPath = "../shaders/phoenix.vert";
std::string Phoenix::fragmentShaderPath = "../shaders/phoenix.frag";