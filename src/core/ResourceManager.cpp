#include "ResourceManager.hpp"
#include <iostream>

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

EntityManager& ResourceManager::entityManager = EntityManager::getInstance();

//-------------------------------------------------------------------------------
//------------------- EntityManager Implementation ------------------------------
//-------------------------------------------------------------------------------
EntityManager& EntityManager::getInstance() {
    static EntityManager instance;
    return instance;
}


EntityID EntityManager::registerEntity(std::shared_ptr<Entity> entity) {
    // Ensure entity isn't null
    if (!entity) {
        std::cerr << "Error: Attempted to register a null entity.\n";
        return -1;
    }
    EntityID id;
    if (!freeEntityIDs.empty()) {
        id = freeEntityIDs.front();
        freeEntityIDs.pop();
        entities[id] = entity;
    } else {
        id = nextEntityID;
        nextEntityID++;
        entities.push_back(entity);
    }
    return id;
}

EntityID EntityManager::registerUpdateableEntity(std::shared_ptr<DynamicEntity> entity) {
    if (!entity) {
        std::cerr << "Error: Attempted to register a null updateable entity.\n";
        return -1;
    }
    EntityID id = registerEntity(std::static_pointer_cast<Entity>(entity));
    updateableEntityIDs.push_back(id);
    return id;
}

void EntityManager::unregisterEntity(EntityID id) {
    if (id < 0 || id >= static_cast<EntityID>(entities.size()) || !entities[id]) {
        std::cerr << "Error: Attempted to unregister an invalid or non-existent entity with ID " << id << ".\n";
        return;
    }
    entities[id] = nullptr;
    freeEntityIDs.push(id);

    // Search for updateable entity IDs and remove if found
    auto it = std::find(updateableEntityIDs.begin(), updateableEntityIDs.end(), id);
    if (it != updateableEntityIDs.end()) {
        updateableEntityIDs.erase(it);
    }
    // TODO: Actually this removal can be optimized using a different data structure.
}
// Initialise members
int EntityManager::nextEntityID = 0;
std::queue<EntityID> EntityManager::freeEntityIDs;
std::vector<EntityID> EntityManager::updateableEntityIDs;
std::vector<std::shared_ptr<Entity>> EntityManager::entities;
