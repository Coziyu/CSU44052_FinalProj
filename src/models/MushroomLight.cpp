#include "MushroomLight.hpp"
#include "utils.hpp"
#include <glm/detail/func_geometric.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <functional>

// Static member definitions
std::string MushroomLight::modelDirectory = "../assets/MushroomLight/";
std::string MushroomLight::modelPath = modelDirectory + std::string("/scene.gltf");
std::string MushroomLight::vertexShaderPath = "../shaders/pbr.vert";
std::string MushroomLight::fragmentShaderPath = "../shaders/pbr.frag";

std::shared_ptr<Shader> MushroomLight::sharedShader = nullptr;
tinygltf::Model MushroomLight::sharedModel;
std::vector<PrimitiveObject> MushroomLight::sharedPrimitives;
std::vector<std::shared_ptr<Texture>> MushroomLight::sharedTextures;
std::unordered_map<int, glm::mat4> MushroomLight::sharedLocalMeshTransforms;
std::unordered_map<int, glm::mat4> MushroomLight::sharedGlobalMeshTransforms;
bool MushroomLight::sharedResourcesLoaded = false;

void MushroomLight::loadSharedResources() {
    if (sharedResourcesLoaded) {
        std::cout << "[MushroomLight] Shared resources already loaded, skipping." << std::endl;
        return;
    }

    std::cout << "[MushroomLight] Loading shared resources..." << std::endl;

    // Load the model once
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool res = loader.LoadASCIIFromFile(&sharedModel, &err, &warn, modelPath.c_str());
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cout << "ERR: " << err << std::endl;
    if (!res) {
        std::cerr << "Failed to load glTF: " << modelPath << std::endl;
        return;
    }
    std::cout << "[MushroomLight] Model loaded: " << modelPath << std::endl;

    // Create shared shader once
    sharedShader = std::make_shared<Shader>(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    if (sharedShader->getProgramID() == 0) {
        std::cerr << "[MushroomLight] Failed to load shaders." << std::endl;
        return;
    }

    // Bind model buffers (VAOs, VBOs) once
    // We need a temporary ModelEntity instance to use its bindModel method
    // Or we can duplicate the logic here. Let's use a helper approach.
    
    // Bind buffers for the shared model
    std::map<int, GLuint> vbos;
    for (size_t i = 0; i < sharedModel.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = sharedModel.bufferViews[i];
        int target = bufferView.target;
        const tinygltf::Buffer &buffer = sharedModel.buffers[bufferView.buffer];
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferView.byteLength,
                    &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        vbos[i] = vbo;
    }

    // Bind meshes to VAOs
    const tinygltf::Scene &scene = sharedModel.scenes[sharedModel.defaultScene];
    std::function<void(int)> bindNode = [&](int nodeIndex) {
        const tinygltf::Node &node = sharedModel.nodes[nodeIndex];
        if (node.mesh >= 0 && node.mesh < sharedModel.meshes.size()) {
            tinygltf::Mesh &mesh = sharedModel.meshes[node.mesh];
            for (size_t i = 0; i < mesh.primitives.size(); ++i) {
                tinygltf::Primitive &primitive = mesh.primitives[i];
                tinygltf::Accessor &indexAccessor = sharedModel.accessors[primitive.indices];

                GLuint vao;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                for (auto &attrib : primitive.attributes) {
                    tinygltf::Accessor &accessor = sharedModel.accessors[attrib.second];
                    int byteStride = accessor.ByteStride(sharedModel.bufferViews[accessor.bufferView]);
                    glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

                    int size = 1;
                    if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                        size = accessor.type;
                    }

                    int vaa = -1;
                    if (attrib.first == "POSITION") vaa = 0;
                    if (attrib.first == "NORMAL") vaa = 1;
                    if (attrib.first == "TEXCOORD_0") vaa = 2;
                    if (attrib.first == "JOINTS_0") vaa = 3;
                    if (attrib.first == "WEIGHTS_0") vaa = 4;
                    if (attrib.first == "TANGENT") vaa = 5;
                    if (attrib.first == "TEXCOORD_1") vaa = 6;
                    if (attrib.first == "TEXCOORD_2") vaa = 7;
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
                sharedPrimitives.push_back(primitiveObject);

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

    // Load textures once
    sharedTextures.clear();
    for (size_t ti = 0; ti < sharedModel.textures.size(); ++ti) {
        int source = sharedModel.textures[ti].source;
        if (source < 0 || source >= sharedModel.images.size()) {
            sharedTextures.push_back(nullptr);
            continue;
        }
        const tinygltf::Image &img = sharedModel.images[source];
        std::string imagePath;
        if (!img.uri.empty()) {
            imagePath = modelDirectory + img.uri;
            std::cout << "[MushroomLight] Loading texture: " << imagePath << std::endl;
        } else {
            sharedTextures.push_back(nullptr);
            continue;
        }
        auto tex = std::make_shared<Texture>(imagePath.c_str(), "tex", (GLuint)ti, GL_RGBA, GL_UNSIGNED_BYTE);
        sharedTextures.push_back(tex);
    }

    // Compute static mesh transforms
    std::function<void(int)> computeLocal = [&](int nodeIndex) {
        const tinygltf::Node &node = sharedModel.nodes[nodeIndex];
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
        sharedLocalMeshTransforms[nodeIndex] = transform;
        for (int child : node.children) {
            computeLocal(child);
        }
    };

    std::function<void(int, glm::mat4)> computeGlobal = [&](int nodeIndex, glm::mat4 parent) {
        sharedGlobalMeshTransforms[nodeIndex] = parent * sharedLocalMeshTransforms[nodeIndex];
        const tinygltf::Node &node = sharedModel.nodes[nodeIndex];
        for (int child : node.children) {
            computeGlobal(child, sharedGlobalMeshTransforms[nodeIndex]);
        }
    };

    for (int rootNode : scene.nodes) {
        computeLocal(rootNode);
        computeGlobal(rootNode, glm::mat4(1.0f));
    }

    sharedResourcesLoaded = true;
    std::cout << "[MushroomLight] Shared resources loaded successfully." << std::endl;
}

void MushroomLight::initializeInstance() {
    if (!sharedResourcesLoaded) {
        std::cerr << "[MushroomLight] ERROR: Call loadSharedResources() before initializeInstance()!" << std::endl;
        loadSharedResources();
    }

    // Set instance defaults
    isSkinned = false;
    alwaysLit = false;
    modelTime = 0.0f;
    animationSpeed = 1.0f;
    position = glm::vec3(0, 0, 0);
    scale = 30.0f * glm::vec3(1.0f, 1.0f, 1.0f);
    rotationAngle = 0.0f;
    rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    active = true;

    // Point to shared resources (don't copy, just reference for rendering)
    shader = sharedShader;
    // Note: model, primitiveObjects, textures will be accessed via static members during render
}

void MushroomLight::initialize(bool isSkinned) { 
    // Legacy path - just use shared resources
    initializeInstance();
}

void MushroomLight::update(float dt) { 
    // No animation for mushrooms, nothing to update per-instance
    // (The shared model has no animations)
}

void MushroomLight::render(glm::mat4 cameraMatrix, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane) {
    if (!active || !sharedResourcesLoaded) return;

    sharedShader->use();
    
    // Set camera
    glm::mat4 vp = cameraMatrix;

    // Set model transform for this instance
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, rotationAxis);

    glm::mat4 mvp = vp * modelMatrix;

    sharedShader->setUniMat4("MVP", mvp);
    sharedShader->setUniBool("isSkinned", false);

    // Set light data 
    sharedShader->setUniVec3("lightPosition", lightingParams.lightPosition);
    sharedShader->setUniVec3("lightIntensity", lightingParams.lightIntensity);
    sharedShader->setUniVec3("cameraPos", cameraPos);
    sharedShader->setUniInt("shadowCubemap", 15);
    sharedShader->setUniBool("alwaysLit", alwaysLit);

    // Draw shared model with this instance's transform
    glDisable(GL_CULL_FACE);
    
    // Draw all primitives
    const tinygltf::Scene &scene = sharedModel.scenes[sharedModel.defaultScene];
    std::function<void(int)> drawNode = [&](int nodeIndex) {
        const tinygltf::Node &node = sharedModel.nodes[nodeIndex];
        
        // Set node matrix
        auto it = sharedGlobalMeshTransforms.find(nodeIndex);
        glm::mat4 nodeGlobal = (it != sharedGlobalMeshTransforms.end()) ? it->second : glm::mat4(1.0f);
        sharedShader->setUniMat4("nodeMatrix", nodeGlobal);

        if (node.mesh >= 0 && node.mesh < sharedModel.meshes.size()) {
            tinygltf::Mesh &mesh = sharedModel.meshes[node.mesh];
            for (size_t i = 0; i < mesh.primitives.size(); ++i) {
                // Find the matching PrimitiveObject
                int foundIndex = -1;
                for (size_t j = 0; j < sharedPrimitives.size(); ++j) {
                    if (sharedPrimitives[j].meshIndex == node.mesh && sharedPrimitives[j].primitiveIndex == (int)i) {
                        foundIndex = (int)j;
                        break;
                    }
                }
                if (foundIndex == -1) continue;

                GLuint vao = sharedPrimitives[foundIndex].vao;
                std::map<int, GLuint> &vbos = sharedPrimitives[foundIndex].vbos;

                glBindVertexArray(vao);

                tinygltf::Primitive &primitive = mesh.primitives[i];
                tinygltf::Accessor &indexAccessor = sharedModel.accessors[primitive.indices];

                // Material handling
                int matIndex = primitive.material;
                glm::vec4 baseColorFactor(1.0f);
                float metallicFactor = 1.0f;
                float roughnessFactor = 1.0f;
                glm::vec3 emissiveFactor(0.0f);
                float occlusionStrength = 1.0f;
                int baseColorTex = -1, mrTex = -1, normalTexIdx = -1, occlusionTexIdx = -1, emissiveTexIdx = -1;
                int baseColorUVSet = 0, mrUVSet = 0, normalUVSet = 0, occlusionUVSet = 0, emissiveUVSet = 0;

                if (matIndex >= 0 && matIndex < sharedModel.materials.size()) {
                    auto &mat = sharedModel.materials[matIndex];
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
                    if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) baseColorUVSet = mat.pbrMetallicRoughness.baseColorTexture.texCoord;
                    if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) mrUVSet = mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
                    if (mat.normalTexture.index >= 0) normalUVSet = mat.normalTexture.texCoord;
                    if (mat.occlusionTexture.index >= 0) occlusionUVSet = mat.occlusionTexture.texCoord;
                    if (mat.emissiveTexture.index >= 0) emissiveUVSet = mat.emissiveTexture.texCoord;
                }

                sharedShader->setUniVec4("u_BaseColorFactor", baseColorFactor);
                sharedShader->setUniFloat("u_MetallicFactor", metallicFactor);
                sharedShader->setUniFloat("u_RoughnessFactor", roughnessFactor);
                sharedShader->setUniVec3("u_EmissiveFactor", emissiveFactor);
                sharedShader->setUniFloat("u_OcclusionStrength", occlusionStrength);

                auto setTex = [&](int texIdx, const char* uniformName, const char* flagName){
                    if (texIdx >= 0 && texIdx < sharedTextures.size() && sharedTextures[texIdx]) {
                        sharedTextures[texIdx]->bind();
                        sharedShader->setUniInt(uniformName, sharedTextures[texIdx]->unit);
                        sharedShader->setUniBool(flagName, true);
                    } else {
                        sharedShader->setUniBool(flagName, false);
                    }
                };

                setTex(baseColorTex, "baseColorTex", "hasBaseColorTex");
                setTex(mrTex, "metallicRoughnessTex", "hasMetallicRoughnessTex");
                setTex(normalTexIdx, "normalTex", "hasNormalTex");
                setTex(occlusionTexIdx, "occlusionTex", "hasOcclusionTex");
                setTex(emissiveTexIdx, "emissiveTex", "hasEmissiveTex");

                sharedShader->setUniInt("baseColorUV", baseColorUVSet);
                sharedShader->setUniInt("mrUV", mrUVSet);
                sharedShader->setUniInt("normalUV", normalUVSet);
                sharedShader->setUniInt("occlusionUV", occlusionUVSet);
                sharedShader->setUniInt("emissiveUV", emissiveUVSet);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
                glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));

                // Unbind textures
                if (baseColorTex >= 0 && baseColorTex < sharedTextures.size() && sharedTextures[baseColorTex]) sharedTextures[baseColorTex]->unbind();
                if (mrTex >= 0 && mrTex < sharedTextures.size() && sharedTextures[mrTex]) sharedTextures[mrTex]->unbind();
                if (normalTexIdx >= 0 && normalTexIdx < sharedTextures.size() && sharedTextures[normalTexIdx]) sharedTextures[normalTexIdx]->unbind();
                if (occlusionTexIdx >= 0 && occlusionTexIdx < sharedTextures.size() && sharedTextures[occlusionTexIdx]) sharedTextures[occlusionTexIdx]->unbind();
                if (emissiveTexIdx >= 0 && emissiveTexIdx < sharedTextures.size() && sharedTextures[emissiveTexIdx]) sharedTextures[emissiveTexIdx]->unbind();

                glBindVertexArray(0);
            }
        }
        for (int child : node.children) {
            drawNode(child);
        }
    };

    for (int rootNode : scene.nodes) {
        drawNode(rootNode);
    }

    glEnable(GL_CULL_FACE);
}

void MushroomLight::renderDepth(std::shared_ptr<Shader> depthShader) {
    if (!active || !sharedResourcesLoaded) return;

    depthShader->use();
    
    // Build model transform
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, rotationAxis);
    
    depthShader->setUniMat4("Model", modelMatrix);
    depthShader->setUniBool("isSkinned", false);
    
    glDisable(GL_CULL_FACE);

    const tinygltf::Scene &scene = sharedModel.scenes[sharedModel.defaultScene];
    std::function<void(int)> drawNode = [&](int nodeIndex) {
        const tinygltf::Node &node = sharedModel.nodes[nodeIndex];
        
        auto it = sharedGlobalMeshTransforms.find(nodeIndex);
        glm::mat4 nodeGlobal = (it != sharedGlobalMeshTransforms.end()) ? it->second : glm::mat4(1.0f);
        depthShader->setUniMat4("nodeMatrix", nodeGlobal);

        if (node.mesh >= 0 && node.mesh < sharedModel.meshes.size()) {
            tinygltf::Mesh &mesh = sharedModel.meshes[node.mesh];
            for (size_t i = 0; i < mesh.primitives.size(); ++i) {
                int foundIndex = -1;
                for (size_t j = 0; j < sharedPrimitives.size(); ++j) {
                    if (sharedPrimitives[j].meshIndex == node.mesh && sharedPrimitives[j].primitiveIndex == (int)i) {
                        foundIndex = (int)j;
                        break;
                    }
                }
                if (foundIndex == -1) continue;

                GLuint vao = sharedPrimitives[foundIndex].vao;
                std::map<int, GLuint> &vbos = sharedPrimitives[foundIndex].vbos;

                glBindVertexArray(vao);

                tinygltf::Primitive &primitive = mesh.primitives[i];
                tinygltf::Accessor &indexAccessor = sharedModel.accessors[primitive.indices];

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
                glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));

                glBindVertexArray(0);
            }
        }
        for (int child : node.children) {
            drawNode(child);
        }
    };

    for (int rootNode : scene.nodes) {
        drawNode(rootNode);
    }

    glEnable(GL_CULL_FACE);
}