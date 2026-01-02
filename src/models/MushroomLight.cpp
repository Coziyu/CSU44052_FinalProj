#include "MushroomLight.hpp"
#include "models/MushroomLight.hpp"
#include <glm/detail/type_vec.hpp>
#include <unordered_map>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>


//-- To change when changing models
void MushroomLight::initialize(bool isSkinned) {
	modelTime = 0.0f;

	position = glm::vec3(0.0f, 0.0f, 0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	// Modify your path if needed
	if (!loadModel(model, "../assets/bot.gltf")) {
		return;
	}

	// Prepare buffers for rendering 
	primitiveObjects = bindModel(model);

	// Prepare mesh transforms for when skinning is not used
	updateMeshTransforms();


	// Prepare joint matrices
	skinObjects = prepareSkinning(model);

	// Prepare animation data 
	animationObjects = prepareAnimation(model);

	// Create and compile our GLSL program from the shaders

	shader = std::make_shared<Shader>("../shaders/mushroom_light.vert", "../shaders/mushroom_light.frag");

	if (shader->getProgramID() == 0) {
		std::cerr << "Failed to load shaders." << std::endl;
	}
}

void MushroomLight::render(glm::mat4 cameraMatrix) {
    shader->use();
    
    // Set camera
    glm::mat4 vp = cameraMatrix;

	// Set model
	glm::mat4 modelMatrix = glm::mat4();
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);

	glm::mat4 mvp = vp * modelMatrix;

    shader->setUniMat4("MVP", mvp);
	shader->setUniBool("isSkinned", isSkinned);
    // -----------------------------------------------------------------
    // TODO: Set animation data for linear blend skinning in shader
    // -----------------------------------------------------------------
    
    if (!skinObjects.empty()) {
        shader->setUniMat4Arr("jointMatrices", skinObjects[0].jointMatrices, skinObjects[0].jointMatrices.size());
    }
	// if (!isSkinned){
	// 	shader->setUniMat4Arr("jointMatrices", localMeshTransforms, localMeshTransforms.size());
	// }
    
    // -----------------------------------------------------------------

    // Set light data 
    shader->setUniVec3("lightPosition", lightPosition);
    shader->setUniVec3("lightIntensity", lightIntensity);

    // Draw the GLTF model
    drawModel(primitiveObjects, model);
}



//-- To check model specific requirements
std::vector<SkinObject> MushroomLight::prepareSkinning(const tinygltf::Model &model) {
	std::vector<SkinObject> skinObjects;

	// In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

	for (size_t i = 0; i < model.skins.size(); i++) {
		SkinObject skinObject;

		const tinygltf::Skin &skin = model.skins[i];
		
		// Read inverseBindMatrices
		const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
		assert(accessor.type == TINYGLTF_TYPE_MAT4);
		const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
		const float *ptr = reinterpret_cast<const float *>(
            	buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);
		
		skinObject.inverseBindMatrices.resize(accessor.count);
		for (size_t j = 0; j < accessor.count; j++) {
			float m[16];
			memcpy(m, ptr + j * 16, 16 * sizeof(float));
			skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
		}

		assert(skin.joints.size() == accessor.count);

		skinObject.globalJointTransforms.resize(skin.joints.size());
		skinObject.jointMatrices.resize(skin.joints.size());

		// ----------------------------------------------
		// TODO: your code here to compute joint matrices
		// ----------------------------------------------

		int rootNodeIndex = skin.joints[0];
		std::unordered_map<int, glm::mat4> localJointTranforms(skin.joints.size());
		computeLocalNodeTransform(model, rootNodeIndex, localJointTranforms);

		std::unordered_map<int, glm::mat4> globalJointTransforms(skin.joints.size());
		computeGlobalNodeTransform(model, localJointTranforms, rootNodeIndex, glm::mat4(1.0f), globalJointTransforms);
		
		for (size_t j = 0; j < skin.joints.size(); j++){
			int nodeIndex = skin.joints[j];
			skinObject.globalJointTransforms[j] = globalJointTransforms[nodeIndex];
			skinObject.jointMatrices[j] =
				skinObject.globalJointTransforms[j] * skinObject.inverseBindMatrices[j];
		}

		// ----------------------------------------------

		skinObjects.push_back(skinObject);
	}
	return skinObjects;
}


std::vector<AnimationObject> MushroomLight::prepareAnimation(const tinygltf::Model &model) 
{
	std::vector<AnimationObject> animationObjects;
	for (const auto &anim : model.animations) {
		AnimationObject animationObject;
		
		for (const auto &sampler : anim.samplers) {
			SamplerObject samplerObject;

			const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
			const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
			const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

			assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

			// Input (time) values
			samplerObject.input.resize(inputAccessor.count);

			const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
			const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

			// Read input (time) values
			int stride = inputAccessor.ByteStride(inputBufferView);
			for (size_t i = 0; i < inputAccessor.count; ++i) {
				samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
			}
			
			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

			int outputStride = outputAccessor.ByteStride(outputBufferView);
			
			// Output values
			samplerObject.output.resize(outputAccessor.count);
			
			for (size_t i = 0; i < outputAccessor.count; ++i) {

				if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
					memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
				} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
					memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
				} else {
					std::cout << "Unsupport accessor type ..." << std::endl;
				}

			}

			animationObject.samplers.push_back(samplerObject);			
		}

		animationObjects.push_back(animationObject);
	}
	return animationObjects;
}

void MushroomLight::updateAnimation(
              		const tinygltf::Model &model, 
              		const tinygltf::Animation &anim, 
              		const AnimationObject &animationObject, 
              		float time,
              		std::unordered_map<int, glm::mat4> &nodeTransforms,
              		bool interpolated
              	) 
{
	// There are many channels so we have to accumulate the transforms 
	for (const auto &channel : anim.channels) {
		
		int targetNodeIndex = channel.target_node;
		const auto &sampler = anim.samplers[channel.sampler];
		
		// Access output (value) data for the channel
		const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
		const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
		const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

		// Calculate current animation time (wrap if necessary)
		const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
		float animationTime = fmod(time, times.back());
		
		// ----------------------------------------------------------
		// keyframe for getting animation data 
		// this keyframe points to the previous keyframe.
		// ----------------------------------------------------------
		int keyframeIndex = findKeyframeIndex(times, animationTime);

		const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
		const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

		// -----------------------------------------------------------
		// Linear interpolation for smooth interpolation
		// -----------------------------------------------------------
		int nextFrameIndex = glm::min(keyframeIndex + 1, static_cast<int>(times.size() - 1));
		float prevFrameTime = times[keyframeIndex];
		float nextFrameTime = times[nextFrameIndex];
		float _t = (animationTime - prevFrameTime) / (nextFrameTime - prevFrameTime);
		
		if (channel.target_path == "translation") {
			glm::vec3 translation0, translation1;
			memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			
			glm::vec3 translation = translation0;
			if (interpolated) {
				memcpy(&translation1, outputPtr + nextFrameIndex * 3 * sizeof(float), 3 * sizeof(float));
				translation = (1 - _t) * translation0 + (_t) * translation1;
			}
			nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
		} else if (channel.target_path == "rotation") {
			glm::quat rotation0, rotation1;
			memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));

			glm::quat rotation = rotation0;
			if (interpolated) {
				memcpy(&rotation1, outputPtr + nextFrameIndex * 4 * sizeof(float), 4 * sizeof(float));
				rotation = glm::slerp(rotation0, rotation1, _t);
			}
			nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
		} else if (channel.target_path == "scale") {
			glm::vec3 scale0, scale1;
			memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

			glm::vec3 scale = scale0;
			if (interpolated) {
				memcpy(&scale1, outputPtr + nextFrameIndex * 3 * sizeof(float), 3 * sizeof(float));
				scale = (1 - _t) * scale0 + (_t) * scale1;
			}
			nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
		}
	}
}

void MushroomLight::updateSkinning(std::unordered_map<int, glm::mat4> &nodeTransforms) {

	// -------------------------------------------------
	// TODO: Recompute joint matrices 
	// -------------------------------------------------
	SkinObject &skinObject = skinObjects[0];
	// Assuming we have updated global?? node transforms here (from updateAnimation)
	tinygltf::Skin &skin = model.skins[0];
	for (int j = 0; j < skin.joints.size(); j++) {
		//! JOINTS STORES NODE INDICES, SO MUST ACCESS THEM THIS WAY.
		int nodeIndex = skin.joints[j];
		skinObject.globalJointTransforms[j] = nodeTransforms[nodeIndex];
		skinObject.jointMatrices[j] = skinObject.globalJointTransforms[j] * skinObject.inverseBindMatrices[j];
	}


}

void MushroomLight::update(float deltaTime) {

	modelTime += deltaTime;
	
	if (model.animations.size() > 0) {
		const tinygltf::Animation &animation = model.animations[0];
		const AnimationObject &animationObject = animationObjects[0];
		
		const tinygltf::Skin &skin = model.skins[0];
		std::unordered_map<int, glm::mat4> nodeTransforms(skin.joints.size());
		for (size_t i = 0; i < nodeTransforms.size(); ++i) {
			nodeTransforms[i] = glm::mat4(1.0);
		}
		
		updateAnimation(model, animation, animationObject, modelTime, nodeTransforms, true);
		
		// Local transforms are already changed. Use it to recompute globals
		// Compute global transforms at each node
		int rootNode = skin.joints[0];
		glm::mat4 parentTransform(1.0f);
		std::unordered_map<int, glm::mat4> globalNodeTransforms(skin.joints.size());
		computeGlobalNodeTransform(model, nodeTransforms, rootNode, glm::mat4(1.0), globalNodeTransforms);
		updateSkinning(globalNodeTransforms);
	}
	updateMeshTransforms();
}

bool MushroomLight::loadModel(tinygltf::Model &model, const char *filename) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
}

void MushroomLight::bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

	std::map<int, GLuint> vbos;
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];

		int target = bufferView.target;
		
		if (bufferView.target == 0) { 
			// The bufferView with target == 0 in our model refers to 
			// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes. 
			// So it is considered safe to skip the warning.
			//std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(target, vbo);
		glBufferData(target, bufferView.byteLength,
					&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
		
		vbos[i] = vbo;
	}

	// Each mesh can contain several primitives (or parts), each we need to 
	// bind to an OpenGL vertex array object
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) vaa = 0;
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
			if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
			if (attrib.first.compare("TANGENT") == 0) vaa = 5;
			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
									accessor.normalized ? GL_TRUE : GL_FALSE,
									byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else {
				std::cout << "vaa missing: " << attrib.first << std::endl;
			}
		}

		// Record VAO for later use
		PrimitiveObject primitiveObject;
		primitiveObject.vao = vao;
		primitiveObject.vbos = vbos;
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
	}
}




void MushroomLight::updateMeshTransforms() {
	int rootNodeIndex = model.scenes[model.defaultScene].nodes[0];
	computeLocalNodeTransform(
		model, 
		rootNodeIndex, 
		localMeshTransforms
	);

	computeGlobalNodeTransform(
		model, 
		localMeshTransforms, 
		rootNodeIndex, 
		glm::mat4(1.0f), 
		globalMeshTransforms
	);
}

//-- Fixed methods
glm::mat4 MushroomLight::getNodeTransform(const tinygltf::Node& node) {
	glm::mat4 transform(1.0f); 

	if (node.matrix.size() == 16) {
		transform = glm::make_mat4(node.matrix.data());
	} else {
		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}
		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
	}
	return transform;
}
void MushroomLight::computeLocalNodeTransform(const tinygltf::Model& model, 
              		int nodeIndex, 
              		std::unordered_map<int, glm::mat4> &localTransforms
              	)
{
	const tinygltf::Node &currNode = model.nodes[nodeIndex];
	localTransforms[nodeIndex] = getNodeTransform(currNode);
	for (int childIndex : currNode.children) {
		computeLocalNodeTransform(model, childIndex, localTransforms);
	}
}

void MushroomLight::computeGlobalNodeTransform(const tinygltf::Model& model, 
		std::unordered_map<int, glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform, 
		std::unordered_map<int, glm::mat4> &globalTransforms)
{
	const tinygltf::Node &currNode = model.nodes[nodeIndex];
	std::string nameNode = currNode.name;
	globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];
	glm::vec4 pos = globalTransforms[nodeIndex] * glm::vec4(0,0,0,1);
	for (int childIndex : currNode.children) {
		computeGlobalNodeTransform(model, localTransforms, childIndex, globalTransforms[nodeIndex], globalTransforms);
	}
}

int MushroomLight::findKeyframeIndex(const std::vector<float>& times, float animationTime) 
{
	int left = 0;
	int right = times.size() - 1;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
			return mid;
		}
		else if (times[mid] > animationTime) {
			right = mid - 1;
		}
		else { // animationTime >= times[mid + 1]
			left = mid + 1;
		}
	}

	// Target not found
	return times.size() - 2;
}

void MushroomLight::bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects, 
						tinygltf::Model &model,
						tinygltf::Node &node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}

std::vector<PrimitiveObject> MushroomLight::bindModel(tinygltf::Model &model) {
	std::vector<PrimitiveObject> primitiveObjects;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}

	return primitiveObjects;
}

void MushroomLight::drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {
	
	for (size_t i = 0; i < mesh.primitives.size(); ++i) 
	{
		GLuint vao = primitiveObjects[i].vao;
		std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

		glBindVertexArray(vao);

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					indexAccessor.componentType,
					BUFFER_OFFSET(indexAccessor.byteOffset));

		glBindVertexArray(0);
	}
}

void MushroomLight::drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, tinygltf::Node &node) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}

void MushroomLight::drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
	// Draw all nodes
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}
}
