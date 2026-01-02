#ifndef MUSHROOMLIGHT_HPP
#define MUSHROOMLIGHT_HPP

#include "Entities.hpp"
#include "Shader.hpp"
#include "utils.hpp"
#include "Loadable.hpp"

#include <glm/detail/type_mat.hpp>
#include <tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);



struct MushroomLight : public Entity {
	// 
	std::shared_ptr<Shader> shader;
	float modelTime;
	bool isSkinned;

	GLuint jointMatricesID;

	tinygltf::Model model;

	std::vector<PrimitiveObject> primitiveObjects;
	std::vector<SkinObject> skinObjects;
	std::vector<AnimationObject> animationObjects;

	std::vector<glm::mat4> localMeshTransforms; // This is a misnomer, because we include nodes without meshes too
	std::vector<glm::mat4> globalMeshTransforms;

	void updateMeshTransforms();

	glm::mat4 getNodeTransform(const tinygltf::Node& node);

	void computeLocalNodeTransform(const tinygltf::Model& model, 
		int nodeIndex, 
		std::vector<glm::mat4> &localTransforms
	);

	void computeGlobalNodeTransform(const tinygltf::Model& model, 
		const std::vector<glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform, 
		std::vector<glm::mat4> &globalTransforms
	);

	std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);

	int findKeyframeIndex(const std::vector<float>& times, float animationTime);

	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);

	void updateAnimation(
		const tinygltf::Model &model, 
		const tinygltf::Animation &anim, 
		const AnimationObject &animationObject, 
		float time,
		std::vector<glm::mat4> &nodeTransforms,
		bool interpolated
	);

	void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);

	void update(float deltaTime);

	bool loadModel(tinygltf::Model &model, const char *filename);

	void initialize(bool isSkinned);

	void bindMesh(
		std::vector<PrimitiveObject> &primitiveObjects,
		tinygltf::Model &model, tinygltf::Mesh &mesh
	);

	void bindModelNodes(
		std::vector<PrimitiveObject> &primitiveObjects, 
		tinygltf::Model &model,
		tinygltf::Node &node
	);

	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

	void drawMesh(
		const std::vector<PrimitiveObject> &primitiveObjects,
		tinygltf::Model &model, 
		tinygltf::Mesh &mesh
	);

	void drawModelNodes(
		const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model &model, 
		tinygltf::Node &node
	);
	void drawModel(
		const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model &model
	);

	void render(glm::mat4 cameraMatrix);

	void cleanup() {
	}
}; 

#endif // MUSHROOMLIGHT_HPP
