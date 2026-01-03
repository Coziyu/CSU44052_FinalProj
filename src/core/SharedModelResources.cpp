#include "SharedModelResources.hpp"

bool SharedModelResources::load(bool prepareSkinningData) {
    if (loaded) {
        std::cout << "[SharedModelResources] Already loaded: " << modelPath << std::endl;
        return true;
    }

    std::cout << "[SharedModelResources] Loading: " << modelPath << std::endl;

    // Load the glTF model
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, modelPath.c_str());
    
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cout << "ERR: " << err << std::endl;
    if (!res) {
        std::cerr << "[SharedModelResources] Failed to load glTF: " << modelPath << std::endl;
        return false;
    }

    // Compile shader
    shader = std::make_shared<Shader>(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    if (shader->getProgramID() == 0) {
        std::cerr << "[SharedModelResources] Failed to compile shader for: " << modelPath << std::endl;
        return false;
    }

    // Bind VAOs/VBOs
    bindModelBuffers();

    // Load textures
    loadTextures();

    // Compute static transforms
    computeStaticTransforms();

    // Prepare skinning/animation if requested
    if (prepareSkinningData && model.skins.size() > 0) {
        this->prepareSkinningData();
        prepareAnimationData();
    }

    loaded = true;
    std::cout << "[SharedModelResources] Loaded successfully: " << modelPath << std::endl;
    return true;
}

void SharedModelResources::bindModelBuffers() {
    // Create VBOs for all buffer views
    std::map<int, GLuint> vbos;
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = model.bufferViews[i];
        int target = bufferView.target;
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferView.byteLength,
                    &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        vbos[i] = vbo;
    }

    // Recursively bind all mesh nodes
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    
    std::function<void(int)> bindNode = [&](int nodeIndex) {
        const tinygltf::Node &node = model.nodes[nodeIndex];
        
        if (node.mesh >= 0 && node.mesh < (int)model.meshes.size()) {
            tinygltf::Mesh &mesh = model.meshes[node.mesh];
            
            for (size_t i = 0; i < mesh.primitives.size(); ++i) {
                tinygltf::Primitive &primitive = mesh.primitives[i];

                GLuint vao;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                for (auto &attrib : primitive.attributes) {
                    tinygltf::Accessor &accessor = model.accessors[attrib.second];
                    int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                    glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

                    int size = 1;
                    if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                        size = accessor.type;
                    }

                    int vaa = -1;
                    if (attrib.first == "POSITION") vaa = 0;
                    else if (attrib.first == "NORMAL") vaa = 1;
                    else if (attrib.first == "TEXCOORD_0") vaa = 2;
                    else if (attrib.first == "JOINTS_0") vaa = 3;
                    else if (attrib.first == "WEIGHTS_0") vaa = 4;
                    else if (attrib.first == "TANGENT") vaa = 5;
                    else if (attrib.first == "TEXCOORD_1") vaa = 6;
                    else if (attrib.first == "TEXCOORD_2") vaa = 7;
                    
                    if (vaa > -1) {
                        glEnableVertexAttribArray(vaa);
                        glVertexAttribPointer(vaa, size, accessor.componentType,
                                            accessor.normalized ? GL_TRUE : GL_FALSE,
                                            byteStride, BUFFER_OFFSET(accessor.byteOffset));
                    }
                }

                PrimitiveObject primitiveObject;
                primitiveObject.vao = vao;
                primitiveObject.vbos = vbos;
                primitiveObject.meshIndex = node.mesh;
                primitiveObject.primitiveIndex = i;
                primitives.push_back(primitiveObject);

                glBindVertexArray(0);
            }
        }
        
        for (int child : node.children) {
            bindNode(child);
        }
    };

    for (int rootNode : scene.nodes) {
        bindNode(rootNode);
    }
}

void SharedModelResources::loadTextures() {
    textures.clear();
    for (size_t ti = 0; ti < model.textures.size(); ++ti) {
        int source = model.textures[ti].source;
        if (source < 0 || source >= (int)model.images.size()) {
            textures.push_back(nullptr);
            continue;
        }
        const tinygltf::Image &img = model.images[source];
        if (!img.uri.empty()) {
            std::string imagePath = modelDirectory + img.uri;
            auto tex = std::make_shared<Texture>(imagePath.c_str(), "tex", (GLuint)ti, GL_RGBA, GL_UNSIGNED_BYTE);
            textures.push_back(tex);
        } else {
            textures.push_back(nullptr);
        }
    }
}

glm::mat4 SharedModelResources::getNodeTransform(const tinygltf::Node& node) const {
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

void SharedModelResources::computeStaticTransforms() {
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];

    std::function<void(int)> computeLocal = [&](int nodeIndex) {
        localMeshTransforms[nodeIndex] = getNodeTransform(model.nodes[nodeIndex]);
        for (int child : model.nodes[nodeIndex].children) {
            computeLocal(child);
        }
    };

    std::function<void(int, glm::mat4)> computeGlobal = [&](int nodeIndex, glm::mat4 parent) {
        globalMeshTransforms[nodeIndex] = parent * localMeshTransforms[nodeIndex];
        for (int child : model.nodes[nodeIndex].children) {
            computeGlobal(child, globalMeshTransforms[nodeIndex]);
        }
    };

    for (int rootNode : scene.nodes) {
        computeLocal(rootNode);
        computeGlobal(rootNode, glm::mat4(1.0f));
    }
}

void SharedModelResources::prepareSkinningData() {
    skinObjects.clear();
    
    for (size_t i = 0; i < model.skins.size(); i++) {
        SkinObject skinObject;
        const tinygltf::Skin &skin = model.skins[i];
        
        // Read inverseBindMatrices
        const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
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

        skinObject.globalJointTransforms.resize(skin.joints.size());
        skinObject.jointMatrices.resize(skin.joints.size());

        // Compute initial joint matrices
        for (size_t j = 0; j < skin.joints.size(); j++) {
            int nodeIndex = skin.joints[j];
            skinObject.globalJointTransforms[j] = globalMeshTransforms[nodeIndex];
            skinObject.jointMatrices[j] = skinObject.globalJointTransforms[j] * skinObject.inverseBindMatrices[j];
        }

        skinObjects.push_back(skinObject);
    }
}

void SharedModelResources::prepareAnimationData() {
    animationObjects.clear();
    
    for (const auto &anim : model.animations) {
        AnimationObject animationObject;
        
        for (const auto &sampler : anim.samplers) {
            SamplerObject samplerObject;

            const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
            const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
            const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

            // Input (time) values
            samplerObject.input.resize(inputAccessor.count);
            const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
            int stride = inputAccessor.ByteStride(inputBufferView);
            for (size_t i = 0; i < inputAccessor.count; ++i) {
                samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
            }
            
            const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
            const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
            const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

            const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
            
            // Output values
            samplerObject.output.resize(outputAccessor.count);
            for (size_t i = 0; i < outputAccessor.count; ++i) {
                if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                    memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
                } else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
                    memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
                }
            }

            animationObject.samplers.push_back(samplerObject);
        }

        animationObjects.push_back(animationObject);
    }
}
