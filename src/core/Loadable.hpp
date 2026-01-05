#ifndef LOADABLE_HPP
#define LOADABLE_HPP

#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

// Each VAO corresponds to each mesh primitive in the GLTF model
struct PrimitiveObject {
	GLuint vao;
	std::map<int, GLuint> vbos;
	int meshIndex;
	int primitiveIndex;
};
// Skinning 
struct SkinObject {
	// Transforms the geometry into the space of the respective joint
	std::vector<glm::mat4> inverseBindMatrices;  

	// Transforms the geometry following the movement of the joints
	std::vector<glm::mat4> globalJointTransforms;

	// Combined transforms
	std::vector<glm::mat4> jointMatrices;
};
// Animation 
struct SamplerObject {
	std::vector<float> input;
	std::vector<glm::vec4> output;
	int interpolation;
};
struct ChannelObject {
	int sampler;
	std::string targetPath;
	int targetNode;
}; 
struct AnimationObject {
	std::vector<SamplerObject> samplers;	// Animation data
};

#endif // LOADABLE_HPP
