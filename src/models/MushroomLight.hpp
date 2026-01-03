#ifndef MUSHROOMLIGHT_HPP
#define MUSHROOMLIGHT_HPP

#include "ModelEntity.hpp"
#include "core/SharedModelResources.hpp"

/**
 * @brief MushroomLight entity with shared resource loading.
 * 
 * Uses static shared resources (model, shader, primitives, textures) to avoid
 * reloading the same data for every instance. Call loadSharedResources() once
 * before creating any instances, then use initializeInstance() for each mushroom.
 */
struct MushroomLight : public ModelEntity {
    // Shared resources - loaded once, used by all instances
    static SharedModelResources sharedResources;
    static bool resourcesInitialized;

    MushroomLight() : ModelEntity() {};

    /**
     * @brief Load shared resources (model, shader, textures) once for all instances.
     * Call this once before creating any MushroomLight instances.
     */
    static void loadSharedResources();

    /**
     * @brief Initialize this instance using shared resources. Lightweight.
     * Sets up per-instance state (position, rotation, scale) without reloading model/shader.
     */
    void initializeInstance(bool skinned = false);

    /**
     * @brief Legacy initialize - uses shared resources.
     */
    void initialize(bool isSkinned);

    void update(float dt);
}; 

#endif // MUSHROOMLIGHT_HPP
