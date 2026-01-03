#ifndef ARCHTREE_HPP
#define ARCHTREE_HPP

#include "ModelEntity.hpp"
#include "SharedModelResources.hpp"

struct ArchTree : public ModelEntity {
    // Shared resources for all ArchTree instances
    static SharedModelResources sharedResources;
    static bool resourcesInitialized;

    ArchTree() : ModelEntity() {};
    
    /**
     * @brief Load shared resources once for all ArchTree instances
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

#endif // ARCHTREE_HPP
