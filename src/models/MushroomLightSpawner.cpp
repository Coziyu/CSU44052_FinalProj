#include "MushroomLightSpawner.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <iostream>

MushroomLightSpawner::MushroomLightSpawner()
    : terrain(nullptr)
    , spawnHeightThreshold(-100.0f)
    , cellSize(150.0f)
    , spawnRadius(2000.0f)
    , despawnRadius(2500.0f)
    , spawnProbability(0.3f)
{
}

void MushroomLightSpawner::initialize(Terrain* terrain, 
                                       float spawnHeightThreshold,
                                       float cellSize,
                                       float spawnRadius,
                                       float despawnRadius,
                                       float spawnProbability) 
{
    this->terrain = terrain;
    this->spawnHeightThreshold = spawnHeightThreshold;
    this->cellSize = cellSize;
    this->spawnRadius = spawnRadius;
    this->despawnRadius = despawnRadius;
    this->spawnProbability = spawnProbability;
    
    allMushrooms.clear();
    cellToMushroomIndex.clear();
    evaluatedCells.clear();
    
    // Load shared resources (model, shader, textures) ONCE for all instances
    MushroomLight::loadSharedResources();
    
    std::cout << "[MushroomLightSpawner] Initialized with threshold=" << spawnHeightThreshold 
              << ", cellSize=" << cellSize << ", spawnRadius=" << spawnRadius << std::endl;
}
// ChatGPT was referred to generate this method
int64_t MushroomLightSpawner::cellKey(int cellX, int cellZ) const {
    // Combine cell coordinates into a unique 64-bit key
    return static_cast<int64_t>(cellX) + static_cast<int64_t>(cellZ) * 1000003LL;
}

void MushroomLightSpawner::worldToCell(float worldX, float worldZ, int& cellX, int& cellZ) const {
    cellX = static_cast<int>(std::floor(worldX / cellSize));
    cellZ = static_cast<int>(std::floor(worldZ / cellSize));
}

// ChatGPT was referred to generate this method
float MushroomLightSpawner::cellRandom(int cellX, int cellZ, int salt) const {
    // Simple but deterministic hash-based PRNG
    uint32_t seed = static_cast<uint32_t>(cellX * 374761393 + cellZ * 668265263 + salt * 1013904223);
    seed = (seed ^ (seed >> 13)) * 1274126177;
    seed = seed ^ (seed >> 16);
    return static_cast<float>(seed & 0x7FFFFFFF) / static_cast<float>(0x7FFFFFFF);
}

bool MushroomLightSpawner::tryActivateCell(int cellX, int cellZ) {
    int64_t key = cellKey(cellX, cellZ);
    
    // Check if we already have a mushroom for this cell
    auto it = cellToMushroomIndex.find(key);
    if (it != cellToMushroomIndex.end()) {
        // Mushroom exists - just activate it
        allMushrooms[it->second]->setActive(true);
        return true;
    }
    
    // Check if we've already evaluated this cell and decided not to spawn
    if (evaluatedCells.count(key) > 0) {
        return false;
    }
    
    // Mark cell as evaluated
    evaluatedCells.insert(key);
    
    // Deterministic spawn probability check
    float spawnRoll = cellRandom(cellX, cellZ, 0);
    if (spawnRoll > spawnProbability) {
        return false;
    }
    
    // Compute deterministic position within the cell
    float jitterX = cellRandom(cellX, cellZ, 1);
    float jitterZ = cellRandom(cellX, cellZ, 2);
    
    float worldX = (cellX + jitterX) * cellSize;
    float worldZ = (cellZ + jitterZ) * cellSize;
    
    // Query terrain height at this position
    float height = terrain->getHeightAt(worldX, worldZ);
    
    // Only spawn if below threshold (in valleys/low areas)
    if (height >= spawnHeightThreshold) {
        return false;
    }
    
    // Get terrain normal for orientation
    glm::vec3 normal = terrain->getNormalAt(worldX, worldZ);
    glm::vec3 position(worldX, height, worldZ);
    
    // Create new mushroom instance (lightweight - uses shared resources)
    auto mushroom = std::make_shared<MushroomLight>();
    mushroom->initializeInstance(false);  // false = not skinned
    configureMushroom(mushroom, position, normal);
    
    // Add to pool
    size_t index = allMushrooms.size();
    allMushrooms.push_back(mushroom);
    cellToMushroomIndex[key] = index;
    
    return true;
}

void MushroomLightSpawner::configureMushroom(std::shared_ptr<MushroomLight> mushroom, 
                                              const glm::vec3& position, 
                                              const glm::vec3& terrainNormal) {
    // mushroom->setPosition(position);
    mushroom->setActive(true);
    
    // Align mushroom's up vector (0, 1, 0) with terrain normal
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 normal = glm::normalize(terrainNormal);
    
    mushroom->setPosition(position -  up * 5.0f); 

    // only rotate if normal differs significantly from up
    float dot = glm::dot(up, normal);
    if (dot < 0.9999f) {
        glm::vec3 axis = glm::cross(up, normal);
        float axisLen = glm::length(axis);
        if (axisLen > 0.0001f) {
            axis = axis / axisLen;
            float angle = std::acos(glm::clamp(dot, -1.0f, 1.0f));
            mushroom->rotationAxis = axis;
            mushroom->rotationAngle = angle;

            // Since the rotation lifts the mushroom off the ground slightly,
            // Translate the muchroom to compensate
            glm::quat rotQuat = glm::angleAxis(angle, axis);
            glm::vec3 adjustedUp = rotQuat * up;
            float heightOffset = 5.0 * glm::dot(adjustedUp - up, normal) * mushroom->getScale().y;
            mushroom->setPosition(position - heightOffset * normal);
        }
    }
}

void MushroomLightSpawner::update(const glm::vec3& cameraPos, float deltaTime) {
    if (!terrain) return;
    
    // 1. Deactivate mushrooms that are too far from camera
    for (auto& mushroom : allMushrooms) {
        if (mushroom->isActive()) {
            float dist = glm::length(mushroom->getPosition() - cameraPos);
            if (dist > despawnRadius) {
                mushroom->setActive(false);
            }
        }
    }
    
    // 2. Activate/spawn mushrooms in nearby cells
    int cameraCellX, cameraCellZ;
    worldToCell(cameraPos.x, cameraPos.z, cameraCellX, cameraCellZ);
    
    int cellRadius = static_cast<int>(std::ceil(spawnRadius / cellSize));
    
    for (int dz = -cellRadius; dz <= cellRadius; ++dz) {
        for (int dx = -cellRadius; dx <= cellRadius; ++dx) {
            int cx = cameraCellX + dx;
            int cz = cameraCellZ + dz;
            
            // Quick distance check (cell center to camera)
            float cellCenterX = (cx + 0.5f) * cellSize;
            float cellCenterZ = (cz + 0.5f) * cellSize;
            float distSq = (cellCenterX - cameraPos.x) * (cellCenterX - cameraPos.x) 
                         + (cellCenterZ - cameraPos.z) * (cellCenterZ - cameraPos.z);
            
            if (distSq <= spawnRadius * spawnRadius) {
                tryActivateCell(cx, cz);
            }
        }
    }
}

size_t MushroomLightSpawner::getActiveMushroomCount() const {
    size_t count = 0;
    for (const auto& m : allMushrooms) {
        if (m->isActive()) count++;
    }
    return count;
}

void MushroomLightSpawner::render(const glm::mat4& vp, const LightingParams& lightingParams, 
                                   const glm::vec3& cameraPos, float farPlane) {
    for (auto& mushroom : allMushrooms) {
        if (mushroom->isActive()) {
            mushroom->render(vp, lightingParams, cameraPos, farPlane);
        }
    }
}

void MushroomLightSpawner::renderDepth(std::shared_ptr<Shader> depthShader) {
    for (auto& mushroom : allMushrooms) {
        if (mushroom->isActive()) {
            mushroom->renderDepth(depthShader);
        }
    }
}
