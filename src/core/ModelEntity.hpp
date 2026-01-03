#ifndef MODELENTITY_HPP
#define MODELENTITY_HPP

#include "Entities.hpp"
#include "Shader.hpp"
#include "utils.hpp"
#include "Loadable.hpp"
#include "LightingParams.hpp"
#include "SharedModelResources.hpp"

#include <glm/detail/type_mat.hpp>
#include <tiny_gltf.h>
#include <unordered_map>
#include "core/Texture.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/**
 * @brief Base class for 3D model entities loaded from glTF files.
 * 
 * Supports two modes:
 * 1. Shared resources mode: Multiple instances share the same GPU resources (model, shader, textures).
 *    Use initializeFromShared() for this mode. Best for spawned/instanced entities.
 * 2. Per-instance mode: Each instance loads its own resources. Use initialize() for this mode.
 *    Best for unique entities or when you need per-instance model data.
 */
struct ModelEntity : public Entity {
	// Per-instance state
	float modelTime;
	float animationSpeed;
	bool isSkinned;
	bool alwaysLit;
	bool active;

	// Reference to shared resources (if using shared mode)
	SharedModelResources* sharedResources;

	// Per-instance resources (if not using shared mode, or for animation state)
	std::shared_ptr<Shader> shader;
	tinygltf::Model model;
	std::vector<PrimitiveObject> primitiveObjects;
	std::vector<std::shared_ptr<Texture>> textures;
	std::vector<SkinObject> skinObjects;
	std::vector<AnimationObject> animationObjects;
	std::unordered_map<int, glm::mat4> localMeshTransforms;
	std::unordered_map<int, glm::mat4> globalMeshTransforms;

	GLuint jointMatricesID;

	ModelEntity();

	// ========== Initialization ==========
	
	/**
	 * @brief Initialize using shared resources (efficient for multiple instances)
	 * @param resources Pointer to pre-loaded SharedModelResources
	 * @param skinned Whether this model uses skeletal animation
	 */
	void initializeFromShared(SharedModelResources* resources, bool skinned = false);

	/**
	 * @brief Initialize loading all resources per-instance (legacy mode)
	 */
	void initialize(bool isSkinned, std::string modelDirectory, std::string modelPath, 
	                std::string vertexShaderPath, std::string fragmentShaderPath);

	// ========== Animation & Skinning ==========
	
	void updateMeshTransforms();
	glm::mat4 getNodeTransform(const tinygltf::Node& node);

	void computeLocalNodeTransform(const tinygltf::Model& model, 
		int nodeIndex, 
		std::unordered_map<int, glm::mat4> &localTransforms
	);

	void computeGlobalNodeTransform(const tinygltf::Model& model, 
		std::unordered_map<int, glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform, 
		std::unordered_map<int, glm::mat4> &globalTransforms);

	std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);
	int findKeyframeIndex(const std::vector<float>& times, float animationTime);
	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);

	void updateAnimation(
		const tinygltf::Model &model, 
		const tinygltf::Animation &anim, 
		const AnimationObject &animationObject, 
		float time,
		std::unordered_map<int, glm::mat4> &nodeTransforms,
		bool interpolated
	);

	void updateSkinning(std::unordered_map<int, glm::mat4> &nodeTransforms);
	void update(float deltaTime);

	// ========== Model Loading (per-instance mode) ==========
	
	bool loadModel(tinygltf::Model &model, const char *filename);

	void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh, int nodeIndex);

	void bindModelNodes(
		std::vector<PrimitiveObject> &primitiveObjects, 
		tinygltf::Model &model,
		tinygltf::Node &node
	);

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

	// ========== Rendering ==========

	void drawMesh(
		const std::vector<PrimitiveObject> &primitiveObjects,
		tinygltf::Model &model, 
		tinygltf::Mesh &mesh,
		int meshIndex
	);

	void drawModelNodes(
		const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model &model, 
		int nodeIndex,
		std::shared_ptr<Shader> shaderToUse = nullptr
	);

	void drawModel(
		const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model &model,
		std::shared_ptr<Shader> shaderToUse = nullptr
	);

	void render(glm::mat4 cameraMatrix, const LightingParams& lightingParams, 
	            glm::vec3 cameraPos, float farPlane = 10000.0f) override;
	void renderDepth(std::shared_ptr<Shader> depthShader);

	// ========== Accessors ==========
	
	void setAlwaysLit(bool lit) { alwaysLit = lit; }
	bool getAlwaysLit() const { return alwaysLit; }
	void setActive(bool a) { active = a; }
	bool isActive() const { return active; }
	bool usesSharedResources() const { return sharedResources != nullptr; }

	void cleanup() {}

protected:
	// Helper to get the active model reference (shared or per-instance)
	tinygltf::Model& getModel();
	const tinygltf::Model& getModel() const;
	std::vector<PrimitiveObject>& getPrimitives();
	const std::vector<PrimitiveObject>& getPrimitives() const;
	std::vector<std::shared_ptr<Texture>>& getTextures();
	std::shared_ptr<Shader> getShader();
	std::unordered_map<int, glm::mat4>& getGlobalMeshTransforms();
}; 

#endif // MODELENTITY_HPP
