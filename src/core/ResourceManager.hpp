#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP
#include "Entities.hpp"
#include <memory>
#include <vector>
#include <queue>

using EntityID = int;
class EntityManager;

/**
 * @brief Singleton class responsible for managing render resources.
 */
class ResourceManager {
    public:
        static ResourceManager& getInstance();
        static EntityManager& entityManager;

        ResourceManager(ResourceManager const&) = delete;
        ResourceManager& operator=(ResourceManager const&) = delete;
        
    private:
        ResourceManager(){};

        // TODO: Add handling of resources for: textures, shaderprograms, buffers, etc.


};

/**
 * @brief Another singleton class responsible for managing entities.
 * 
 */
class EntityManager {
    public:
        static EntityManager& getInstance();

        EntityManager(EntityManager const&) = delete;
        EntityManager& operator=(EntityManager const&) = delete;


        static EntityID registerEntity(std::shared_ptr<Entity> entity);
        static EntityID registerUpdateableEntity(std::shared_ptr<DynamicEntity> entity);
        static void unregisterEntity(EntityID id);
        static std::shared_ptr<Entity> getEntity(EntityID id);


    private:
        EntityManager(){};

    protected:
        /**
         * @brief Table that contains all entities.
         * All entities are drawable.
         */
        static std::vector<std::shared_ptr<Entity>> entities;
        static std::queue<EntityID> freeEntityIDs;
        static int nextEntityID;

        static std::vector<EntityID> updateableEntityIDs;
};


#endif // RESOURCEMANAGER_HPP
