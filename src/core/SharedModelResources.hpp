#ifndef SHAREDMODELRESOURCES_HPP
#define SHAREDMODELRESOURCES_HPP

// Include OpenGL first to avoid header conflicts
#ifndef GL_H
#define GL_H
#include <glad/gl.h>
#include <glfw/glfw3.h>
#endif

#include "Loadable.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "utils.hpp"

#include <tiny_gltf.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/**
 * @brief Holds all GPU resources for a model type, loaded once and shared across all instances.
 * 
 * This includes the glTF model, compiled shader, VAOs/VBOs, textures, and precomputed transforms.
 * Call load() once before creating any instances that use this resource.
 */
struct SharedModelResources {
    // Resource paths
    std::string modelDirectory;
    std::string modelPath;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    // Loaded state
    bool loaded = false;

    // Shared GPU resources
    std::shared_ptr<Shader> shader;
    tinygltf::Model model;
    std::vector<PrimitiveObject> primitives;
    std::vector<std::shared_ptr<Texture>> textures;
    
    // Precomputed static transforms (for non-animated models)
    std::unordered_map<int, glm::mat4> localMeshTransforms;
    std::unordered_map<int, glm::mat4> globalMeshTransforms;

    // Animation data (for animated models)
    std::vector<SkinObject> skinObjects;
    std::vector<AnimationObject> animationObjects;

    SharedModelResources() = default;

    SharedModelResources(const std::string& modelDir, 
                         const std::string& modelFile,
                         const std::string& vertShader,
                         const std::string& fragShader)
        : modelDirectory(modelDir)
        , modelPath(modelFile)
        , vertexShaderPath(vertShader)
        , fragmentShaderPath(fragShader)
        , loaded(false)
    {}

    /**
     * @brief Load all resources (model, shader, textures, buffers). Call once.
     * @param prepareSkinning Whether to prepare skinning data for animated models
     * @return true if successful
     */
    bool load(bool prepareSkinning = false);

    /**
     * @brief Check if resources are loaded
     */
    bool isLoaded() const { return loaded; }

private:
    void bindModelBuffers();
    void loadTextures();
    void computeStaticTransforms();
    void prepareSkinningData();
    void prepareAnimationData();
    
    // Helper to get node transform matrix
    glm::mat4 getNodeTransform(const tinygltf::Node& node) const;
};

#endif // SHAREDMODELRESOURCES_HPP
