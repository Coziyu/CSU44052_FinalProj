#include "ModelEntity.hpp"
#include <glm/detail/type_vec.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "core/Texture.hpp"

#include <tiny_gltf.h>


static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);


//-- To change when changing models
void ModelEntity::initialize(bool isSkinned, std::string modelDirectory, std::string modelPath, std::string vertexShaderPath, std::string fragmentShaderPath) {
	this->isSkinned = isSkinned;
	modelTime = 0.0f;

	position = glm::vec3(500.0f, 150.0f, 500.0f);
	scale = 1.0f * glm::vec3(1.0f, 1.0f, 1.0f);

	// Modify your path if needed
	if (!loadModel(model, modelPath.c_str())) {
		return;
	}
	std::cout << "Loading model: " << modelPath << std::endl;
	std::cout << "model node count" << model.nodes.size() << std::endl;
	std::cout << "joint count" << model.skins[0].joints.size() << std::endl;

	// Prepare buffers for rendering 
	primitiveObjects = bindModel(model);

	// Prepare mesh transforms for when skinning is not used
	updateMeshTransforms();


	// Prepare joint matrices
	skinObjects = prepareSkinning(model);

	// Prepare animation data 
	animationObjects = prepareAnimation(model);

	// Create and compile our GLSL program from the shaders

	shader = std::make_shared<Shader>(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

	if (shader->getProgramID() == 0) {
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Load textures referenced by the glTF model (indexed by model.textures)
	textures.clear();
	for (size_t ti = 0; ti < model.textures.size(); ++ti) {
		int source = model.textures[ti].source;
		if (source < 0 || source >= model.images.size()) {
			textures.push_back(nullptr);
			continue;
		}
		const tinygltf::Image &img = model.images[source];
		std::string imagePath;
		if (!img.uri.empty()) {
			// assume textures live next to the glTF file in ../assets/arch_tree/
			imagePath = modelDirectory + img.uri;
			std::cout << "Loading texture: " << imagePath << std::endl;
		} else {
			// if embedded or buffer view not supported, skip
			textures.push_back(nullptr);
			continue;
		}
		// Create texture at texture unit = ti
		auto tex = std::make_shared<Texture>(imagePath.c_str(), "tex", (GLuint)ti, GL_RGBA, GL_UNSIGNED_BYTE);
		textures.push_back(tex);
	}
}

void ModelEntity::render(glm::mat4 cameraMatrix) {
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
	glDisable(GL_CULL_FACE);
	drawModel(primitiveObjects, model);
	glEnable(GL_CULL_FACE);
}



//-- To check model specific requirements
std::vector<SkinObject> ModelEntity::prepareSkinning(const tinygltf::Model &model) {
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
		std::unordered_map<int, glm::mat4> localJointTranforms(model.nodes.size());
		computeLocalNodeTransform(model, rootNodeIndex, localJointTranforms);

		std::unordered_map<int, glm::mat4> globalJointTransforms(model.nodes.size());
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


std::vector<AnimationObject> ModelEntity::prepareAnimation(const tinygltf::Model &model) 
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

void ModelEntity::updateAnimation(
              		const tinygltf::Model &model, 
              		const tinygltf::Animation &anim, 
              		const AnimationObject &animationObject, 
              		float time,
              		std::unordered_map<int, glm::mat4> &nodeTransforms,
              		bool interpolated
              	) 
{
	// Build per-node TRS (start from node's base TRS values)
	struct TRS { glm::vec3 T; glm::quat R; glm::vec3 S; };
	std::vector<TRS> trs(model.nodes.size());
	for (size_t i = 0; i < model.nodes.size(); ++i) {
		const tinygltf::Node &n = model.nodes[i];
		trs[i].T = (n.translation.size() == 3) ? glm::vec3(n.translation[0], n.translation[1], n.translation[2]) : glm::vec3(0.0f);
		if (n.rotation.size() == 4) trs[i].R = glm::quat(n.rotation[3], n.rotation[0], n.rotation[1], n.rotation[2]);
		else trs[i].R = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		trs[i].S = (n.scale.size() == 3) ? glm::vec3(n.scale[0], n.scale[1], n.scale[2]) : glm::vec3(1.0f);
	}

	// Apply animation channels to TRS
	for (const auto &channel : anim.channels) {
		int targetNodeIndex = channel.target_node;
		const auto &sampler = anim.samplers[channel.sampler];

		const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
		const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
		const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

		const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
		float animationTime = fmod(time, times.back());
		int keyframeIndex = findKeyframeIndex(times, animationTime);
		int nextFrameIndex = glm::min(keyframeIndex + 1, static_cast<int>(times.size() - 1));
		float prevFrameTime = times[keyframeIndex];
		float nextFrameTime = times[nextFrameIndex];
		float _t = (animationTime - prevFrameTime) / (nextFrameTime - prevFrameTime);

		const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];

		if (channel.target_path == "translation") {
			glm::vec3 t0, t1;
			memcpy(&t0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			if (interpolated) {
				memcpy(&t1, outputPtr + nextFrameIndex * 3 * sizeof(float), 3 * sizeof(float));
				trs[targetNodeIndex].T = (1 - _t) * t0 + _t * t1;
			} else {
				trs[targetNodeIndex].T = t0;
			}
		} else if (channel.target_path == "rotation") {
			glm::quat r0, r1;
			memcpy(&r0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
			if (interpolated) {
				memcpy(&r1, outputPtr + nextFrameIndex * 4 * sizeof(float), 4 * sizeof(float));
				trs[targetNodeIndex].R = glm::slerp(r0, r1, _t);
			} else {
				trs[targetNodeIndex].R = r0;
			}
		} else if (channel.target_path == "scale") {
			glm::vec3 s0, s1;
			memcpy(&s0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			if (interpolated) {
				memcpy(&s1, outputPtr + nextFrameIndex * 3 * sizeof(float), 3 * sizeof(float));
				trs[targetNodeIndex].S = (1 - _t) * s0 + _t * s1;
			} else {
				trs[targetNodeIndex].S = s0;
			}
		}
	}

	// Compose local matrices and write back to nodeTransforms
	for (size_t i = 0; i < model.nodes.size(); ++i) {
		glm::mat4 local = glm::mat4(1.0f);
		local = glm::translate(local, trs[i].T);
		local *= glm::mat4_cast(trs[i].R);
		local = glm::scale(local, trs[i].S);
		nodeTransforms[i] = local;
	}
}

void ModelEntity::updateSkinning(std::unordered_map<int, glm::mat4> &nodeTransforms) {

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

void ModelEntity::update(float deltaTime) {

	modelTime += deltaTime;
	
	if (model.animations.size() > 0) {
		const tinygltf::Animation &animation = model.animations[0];
		const AnimationObject &animationObject = animationObjects[0];
		
		const tinygltf::Skin &skin = model.skins[0];
		// Initialize local node transforms from the model (so non-animated nodes keep their base transform)
		std::unordered_map<int, glm::mat4> nodeTransforms(model.nodes.size());
		for (size_t i = 0; i < model.nodes.size(); ++i) {
			nodeTransforms[i] = getNodeTransform(model.nodes[i]);
		}

		// Apply animation channels to local transforms
		updateAnimation(model, animation, animationObject, modelTime, nodeTransforms, true);

		// Compute global transforms for the whole scene (all scene roots)
		std::unordered_map<int, glm::mat4> globalNodeTransforms(model.nodes.size());
		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			int rootNode = scene.nodes[i];
			computeGlobalNodeTransform(model, nodeTransforms, rootNode, glm::mat4(1.0f), globalNodeTransforms);
		}

		// Update skin joint matrices from the computed global transforms
		updateSkinning(globalNodeTransforms);

		// Use animated global transforms for drawing
		globalMeshTransforms = globalNodeTransforms;
	}
	else {
		// No animations: use static transforms from model nodes
		updateMeshTransforms();
	}
}

bool ModelEntity::loadModel(tinygltf::Model &model, const char *filename) {
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

void ModelEntity::bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh, int nodeIndex) {

	std::map<int, GLuint> vbos;
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];

		int target = bufferView.target;
		
		// if (bufferView.target == 0) { 
		// 	// The bufferView with target == 0 in our model refers to 
		// 	// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes. 
		// 	// So it is considered safe to skip the warning.
		// 	//std::cout << "WARN: bufferView.target is zero" << std::endl;
		// 	continue;
		// }

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
			if (attrib.first.compare("TEXCOORD_1") == 0) vaa = 6;
			if (attrib.first.compare("TEXCOORD_2") == 0) vaa = 7;
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
		primitiveObject.meshIndex = nodeIndex;
		primitiveObject.primitiveIndex = i;
		
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
	}
}




void ModelEntity::updateMeshTransforms() {
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
glm::mat4 ModelEntity::getNodeTransform(const tinygltf::Node& node) {
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
void ModelEntity::computeLocalNodeTransform(const tinygltf::Model& model, 
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

void ModelEntity::computeGlobalNodeTransform(const tinygltf::Model& model, 
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

int ModelEntity::findKeyframeIndex(const std::vector<float>& times, float animationTime) 
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

void ModelEntity::bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects, 
						tinygltf::Model &model,
						tinygltf::Node &node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(primitiveObjects, model, model.meshes[node.mesh], node.mesh);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}

std::vector<PrimitiveObject> ModelEntity::bindModel(tinygltf::Model &model) {
	std::vector<PrimitiveObject> primitiveObjects;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}

	return primitiveObjects;
}

void ModelEntity::drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh, int meshIndex) {
    
	for (size_t i = 0; i < mesh.primitives.size(); ++i) 
	{
		// Find the matching PrimitiveObject for this mesh and primitive index
		int foundIndex = -1;
		for (size_t j = 0; j < primitiveObjects.size(); ++j) {
			if (primitiveObjects[j].meshIndex == meshIndex && primitiveObjects[j].primitiveIndex == (int)i) {
				foundIndex = (int)j;
				break;
			}
		}
		if (foundIndex == -1) continue;

		GLuint vao = primitiveObjects[foundIndex].vao;
		std::map<int, GLuint> vbos = primitiveObjects[foundIndex].vbos;

		glBindVertexArray(vao);

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		// Material handling
		int matIndex = primitive.material;
		glm::vec4 baseColorFactor(1.0f);
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec3 emissiveFactor(0.0f);
		float occlusionStrength = 1.0f;
		int baseColorTex = -1, mrTex = -1, normalTexIdx = -1, occlusionTexIdx = -1, emissiveTexIdx = -1;
		int baseColorUVSet = 0, mrUVSet = 0, normalUVSet = 0, occlusionUVSet = 0, emissiveUVSet = 0;

		if (matIndex >= 0 && matIndex < model.materials.size()) {
			auto &mat = model.materials[matIndex];
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				// older tinygltf uses values map; prefer pbrMetallicRoughness if present
				std::cout << "Skipped handling of baseColorFactor" << std::endl;
			}
			if (mat.pbrMetallicRoughness.baseColorFactor.size() == 4) {
				baseColorFactor = glm::vec4(
					mat.pbrMetallicRoughness.baseColorFactor[0],
					mat.pbrMetallicRoughness.baseColorFactor[1],
					mat.pbrMetallicRoughness.baseColorFactor[2],
					mat.pbrMetallicRoughness.baseColorFactor[3]
				);
			}
			metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
			roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
			if (mat.emissiveFactor.size() == 3) {
				emissiveFactor = glm::vec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);
			}
			if (mat.occlusionTexture.index >= 0) occlusionTexIdx = mat.occlusionTexture.index;
			if (mat.normalTexture.index >= 0) normalTexIdx = mat.normalTexture.index;
			if (mat.emissiveTexture.index >= 0) emissiveTexIdx = mat.emissiveTexture.index;
			if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) baseColorTex = mat.pbrMetallicRoughness.baseColorTexture.index;
			if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) mrTex = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;

			// Retrieve explicit texCoord set indices if provided (default 0)
			if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) baseColorUVSet = mat.pbrMetallicRoughness.baseColorTexture.texCoord;
			if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) mrUVSet = mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
			if (mat.normalTexture.index >= 0) normalUVSet = mat.normalTexture.texCoord;
			if (mat.occlusionTexture.index >= 0) occlusionUVSet = mat.occlusionTexture.texCoord;
			if (mat.emissiveTexture.index >= 0) emissiveUVSet = mat.emissiveTexture.texCoord;
		}

		// Set material uniforms
		shader->setUniVec4("u_BaseColorFactor", baseColorFactor);
		shader->setUniFloat("u_MetallicFactor", metallicFactor);
		shader->setUniFloat("u_RoughnessFactor", roughnessFactor);
		shader->setUniVec3("u_EmissiveFactor", emissiveFactor);
		shader->setUniFloat("u_OcclusionStrength", occlusionStrength);

		// Bind and set samplers if present
		auto setTex = [&](int texIdx, const char* uniformName, const char* flagName){
			if (texIdx >= 0 && texIdx < textures.size() && textures[texIdx]) {
				textures[texIdx]->bind();
				shader->setUniInt(uniformName, textures[texIdx]->unit);
				shader->setUniBool(flagName, true);
			} else {
				shader->setUniBool(flagName, false);
			}
		};

		setTex(baseColorTex, "baseColorTex", "hasBaseColorTex");
		setTex(mrTex, "metallicRoughnessTex", "hasMetallicRoughnessTex");
		setTex(normalTexIdx, "normalTex", "hasNormalTex");
		setTex(occlusionTexIdx, "occlusionTex", "hasOcclusionTex");
		setTex(emissiveTexIdx, "emissiveTex", "hasEmissiveTex");

		// Set which UV set each sampler should use (0 = TEXCOORD_0, 1 = TEXCOORD_1, 2 = TEXCOORD_2)
		shader->setUniInt("baseColorUV", baseColorUVSet);
		shader->setUniInt("mrUV", mrUVSet);
		shader->setUniInt("normalUV", normalUVSet);
		shader->setUniInt("occlusionUV", occlusionUVSet);
		shader->setUniInt("emissiveUV", emissiveUVSet);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					indexAccessor.componentType,
					BUFFER_OFFSET(indexAccessor.byteOffset));

		// Unbind textures we bound (optional)
		if (baseColorTex >= 0 && baseColorTex < textures.size() && textures[baseColorTex]) textures[baseColorTex]->unbind();
		if (mrTex >= 0 && mrTex < textures.size() && textures[mrTex]) textures[mrTex]->unbind();
		if (normalTexIdx >= 0 && normalTexIdx < textures.size() && textures[normalTexIdx]) textures[normalTexIdx]->unbind();
		if (occlusionTexIdx >= 0 && occlusionTexIdx < textures.size() && textures[occlusionTexIdx]) textures[occlusionTexIdx]->unbind();
		if (emissiveTexIdx >= 0 && emissiveTexIdx < textures.size() && textures[emissiveTexIdx]) textures[emissiveTexIdx]->unbind();

		glBindVertexArray(0);
	}
}

void ModelEntity::drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, int nodeIndex) {
	// Compute node-specific transform from precomputed global transforms
	auto it = globalMeshTransforms.find(nodeIndex);
	glm::mat4 nodeGlobal = (it != globalMeshTransforms.end()) ? it->second : glm::mat4(1.0f);

	// If this node has a skin, jointMatrices already include the node's global influence,
	// so avoid applying nodeGlobal again (would double-transform). Otherwise set nodeMatrix.
	const tinygltf::Node &node = model.nodes[nodeIndex];
	if (node.skin >= 0) {
		shader->setUniMat4("nodeMatrix", glm::mat4(1.0f));
	} else {
		shader->setUniMat4("nodeMatrix", nodeGlobal);
	}
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(primitiveObjects, model, model.meshes[node.mesh], node.mesh);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(primitiveObjects, model, node.children[i]);
	}
}

void ModelEntity::drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
	// Draw all nodes
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(primitiveObjects, model, scene.nodes[i]);
	}
}
