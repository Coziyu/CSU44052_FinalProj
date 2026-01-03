#include "ArchTree.hpp"

void ArchTree::initialize(bool isSkinned) { ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath); };

std::string ArchTree::modelDirectory = "../assets/arch_tree/";
std::string ArchTree::modelPath = modelDirectory + std::string("/scene.gltf");
std::string ArchTree::vertexShaderPath = "../shaders/pbr.vert";
std::string ArchTree::fragmentShaderPath = "../shaders/pbr.frag";