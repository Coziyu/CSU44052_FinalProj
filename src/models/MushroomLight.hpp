#ifndef MUSHROOMLIGHT_HPP
#define MUSHROOMLIGHT_HPP

#include "ModelEntity.hpp"

/**
 * @brief MushroomLight entity with shared resource loading.
 * 
 * Uses static shared resources (model, shader, primitives, textures) to avoid
 * reloading the same data for every instance. Call loadSharedResources() once
 * before creating any instances, then use initializeInstance() for each mushroom.
 */
struct MushroomLight : public ModelEntity {
    // Shared resource paths
    static std::string modelDirectory;
    static std::string modelPath;
    static std::string vertexShaderPath;
    static std::string fragmentShaderPath;

    // Shared resources - loaded once, used by all instances
    static std::shared_ptr<Shader> sharedShader;
    static tinygltf::Model sharedModel;
    static std::vector<PrimitiveObject> sharedPrimitives;
    static std::vector<std::shared_ptr<Texture>> sharedTextures;
    static std::unordered_map<int, glm::mat4> sharedLocalMeshTransforms;
    static std::unordered_map<int, glm::mat4> sharedGlobalMeshTransforms;
    static bool sharedResourcesLoaded;

    // Instance state
    bool active;

    MushroomLight() : ModelEntity(), active(false) {};

    /**
     * @brief Load shared resources (model, shader, textures) once for all instances.
     * Call this once before creating any MushroomLight instances.
     */
    static void loadSharedResources();

    /**
     * @brief Initialize this instance using shared resources. Lightweight.
     * Sets up per-instance state (position, rotation, scale) without reloading model/shader.
     */
    void initializeInstance();

    /**
     * @brief Legacy initialize - loads everything fresh. Use loadSharedResources() + initializeInstance() instead.
     */
    void initialize(bool isSkinned);

    void update(float dt);

    /**
     * @brief Render using shared resources
     */
    void render(glm::mat4 cameraMatrix, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane = 10000.0f) override;

    /**
     * @brief Render depth pass using shared resources
     */
    void renderDepth(std::shared_ptr<Shader> depthShader);

    // Activation control
    void setActive(bool isActive) { active = isActive; }
    bool isActive() const { return active; }
}; 

#endif // MUSHROOMLIGHT_HPP
