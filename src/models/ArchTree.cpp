#include "ArchTree.hpp"
#include "ModelEntity.hpp"

void ArchTree::initialize(bool isSkinned) { ModelEntity::initialize(isSkinned, modelDirectory, modelPath, vertexShaderPath, fragmentShaderPath); };

std::string ArchTree::modelDirectory = "../assets/arch_tree/";
std::string ArchTree::modelPath = modelDirectory + std::string("/scene.gltf");
std::string ArchTree::vertexShaderPath = "../shaders/pbr.vert";
std::string ArchTree::fragmentShaderPath = "../shaders/pbr.frag";
void ArchTree::update(float dt) {
    static float initialY = position.y;
    // Bob up and down slowly
    float bobHeight = 50.0f;
    float bobSpeed = 0.5f;
    position.y = initialY + bobHeight * cos(modelTime * bobSpeed);

    ModelEntity::update(dt);
}
