#ifndef CHEESEMOON_HPP
#define CHEESEMOON_HPP

#include "ModelEntity.hpp"
#include "SharedModelResources.hpp"

struct CheeseMoon : public ModelEntity {
    // Shared resources for all CheeseMoon instances
    static SharedModelResources sharedResources;
    static bool resourcesInitialized;

    CheeseMoon() : ModelEntity() {};
    
    /**
     * @brief Load shared resources once for all CheeseMoon instances
     */
    static void loadSharedResources();
    
    /**
     * @brief Initialize this instance using shared resources
     */
    void initializeInstance(bool skinned = false);
    
    /**
     * @brief Legacy initialize (loads per-instance, less efficient)
     */
    void initialize(bool isSkinned);
    
    void update(float dt, glm::vec3 cameraPos);
}; 

#endif // CHEESEMOON_HPP
