#include "Entities.hpp"
int Entity::getEntityID() const { return entityID; }
void Entity::setEntityID(int id) {
    if (entityID != -1) {
        // Yellow printing
        std::cout << "\033[33m";
        std::cout << "[WARN] Overwriting existing entity ID " << entityID
                  << " with new ID " << id << ".\n";
        std::cout << "\033[0m";
    }
    entityID = id;
}
