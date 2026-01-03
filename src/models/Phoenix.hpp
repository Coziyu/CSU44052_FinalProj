#ifndef PHOENIX_HPP
#define PHOENIX_HPP

#include "ModelEntity.hpp"
#include "SharedModelResources.hpp"

struct Phoenix : public ModelEntity {
    // Shared resources for all Phoenix instances
    static SharedModelResources sharedResources;
    static bool resourcesInitialized;

    Phoenix() : ModelEntity() {};
    
    /**
     * @brief Load shared resources once for all Phoenix instances
     */
    static void loadSharedResources();
    
    /**
     * @brief Initialize this instance using shared resources
     */
    void initializeInstance(bool skinned = true);
    
    /**
     * @brief Legacy initialize (loads per-instance, less efficient)
     */
    void initialize(bool isSkinned);
    
    void update(float dt);
}; 

#endif // PHOENIX_HPP
