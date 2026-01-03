#ifndef MODELENTITY_HPP
#define MODELENTITY_HPP

#include "Entities.hpp"
#include "Shader.hpp"
#include "utils.hpp"
#include "Loadable.hpp"
#include "LightingParams.hpp"

#include <glm/detail/type_mat.hpp>
#include <tiny_gltf.h>
#include <unordered_map>
#include "core/Texture.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct ModelEntity : public Entity {
	// 
	std::shared_ptr<Shader> shader;
	float modelTime;
	float animationSpeed;
	bool isSkinned;
	bool alwaysLit;

	GLuint jointMatricesID;

	tinygltf::Model model;

	std::vector<PrimitiveObject> primitiveObjects;
	std::vector<std::shared_ptr<Texture>> textures; // textures indexed by glTF texture index
	std::vector<SkinObject> skinObjects;
	std::vector<AnimationObject> animationObjects;

	std::unordered_map<int, glm::mat4> localMeshTransforms; // This is a misnomer, because we include nodes without meshes too
	std::unordered_map<int, glm::mat4> globalMeshTransforms;

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

	bool loadModel(tinygltf::Model &model, const char *filename);

	void initialize(bool isSkinned, std::string modelDirectory, std::string modelPath, std::string vertexShaderPath, std::string fragmentShaderPath);

	void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh, int nodeIndex);

	void bindModelNodes(
		std::vector<PrimitiveObject> &primitiveObjects, 
		tinygltf::Model &model,
		tinygltf::Node &node
	);

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

	void drawMesh(
		const std::vector<PrimitiveObject> &primitiveObjects,
		tinygltf::Model &model, 
		tinygltf::Mesh &mesh,
		int meshIndex
	);

	// shaderToUse override which shader gets the nodeMatrix uniform
	// need for depth rendering so shadows work properly
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

	void renderDepth(std::shared_ptr<Shader> depthShader);

	void setAlwaysLit(bool lit) { alwaysLit = lit; }
	bool getAlwaysLit() const { return alwaysLit; }

	void render(glm::mat4 cameraMatrix, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane = 10000.0f) override;

	void cleanup() {
	}
}; 

#endif // MODELENTITY_HPP
