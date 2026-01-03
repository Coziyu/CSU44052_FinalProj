#ifndef MUSHROOMLIGHTSPAWNER_HPP
#define MUSHROOMLIGHTSPAWNER_HPP

#include "MushroomLight.hpp"
#include "../Terrain.hpp"
#include "../LightingParams.hpp"
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <glm/glm.hpp>

/**
 * @brief Procedurally spawns MushroomLight entities near the camera based on terrain height.
 * 
 * Uses a grid-based deterministic PRNG approach with object pooling:
 * - World is divided into cells of cellSize x cellSize
 * - Each cell gets a deterministic spawn point via PRNG seeded by cell coordinates
 * - Mushrooms spawn only where terrain height < spawnHeightThreshold
 * - Mushrooms are deactivated (not deleted) when camera moves too far away
 * - Returning to an area reactivates the same mushrooms (no reloading needed)
 * - Uses shared resources (model, shader, textures) loaded once for all instances
 */
class MushroomLightSpawner {
public:
    MushroomLightSpawner();

    /**
     * @brief Initialize the spawner with terrain reference and spawn parameters.
     * Also loads shared MushroomLight resources (model, shader, textures) once.
     */
    void initialize(Terrain* terrain, 
                    float spawnHeightThreshold = -100.0f,
                    float cellSize = 150.0f,
                    float spawnRadius = 2000.0f,
                    float despawnRadius = 2500.0f,
                    float spawnProbability = 0.3f);

    /**
     * @brief Update spawner - activate/deactivate mushrooms based on camera position
     */
    void update(const glm::vec3& cameraPos, float deltaTime);

    /**
     * @brief Render all active mushrooms
     */
    void render(const glm::mat4& vp, const LightingParams& lightingParams, 
                const glm::vec3& cameraPos, float farPlane);

    /**
     * @brief Render depth pass for all active mushrooms (for shadow mapping)
     */
    void renderDepth(std::shared_ptr<Shader> depthShader);

    // Accessors for UI/debugging
    size_t getActiveMushroomCount() const;
    size_t getTotalMushroomCount() const { return allMushrooms.size(); }
    void setSpawnHeightThreshold(float threshold) { spawnHeightThreshold = threshold; }
    float getSpawnHeightThreshold() const { return spawnHeightThreshold; }

private:
    Terrain* terrain;
    
    // Spawn parameters
    float spawnHeightThreshold;
    float cellSize;
    float spawnRadius;
    float despawnRadius;
    float spawnProbability;

    // Object pool: all mushrooms ever created (active or inactive)
    std::vector<std::shared_ptr<MushroomLight>> allMushrooms;
    
    // Map from cell key to mushroom index in allMushrooms
    std::unordered_map<int64_t, size_t> cellToMushroomIndex;
    
    // Set of cell keys that have been evaluated (spawned or determined not to spawn)
    std::unordered_set<int64_t> evaluatedCells;

    /**
     * @brief Generate a unique key for a cell from its coordinates
     */
    int64_t cellKey(int cellX, int cellZ) const;

    /**
     * @brief Get cell coordinates from world position
     */
    void worldToCell(float worldX, float worldZ, int& cellX, int& cellZ) const;

    /**
     * @brief Deterministic PRNG based on cell coordinates
     */
    float cellRandom(int cellX, int cellZ, int salt = 0) const;

    /**
     * @brief Try to spawn or reactivate a mushroom in the given cell
     * @return true if mushroom is now active in that cell
     */
    bool tryActivateCell(int cellX, int cellZ);

    /**
     * @brief Configure a mushroom instance at the given position with terrain alignment
     */
    void configureMushroom(std::shared_ptr<MushroomLight> mushroom, 
                           const glm::vec3& position, 
                           const glm::vec3& terrainNormal);
};

#endif // MUSHROOMLIGHTSPAWNER_HPP
