#include "Terrain.hpp"
#include "Camera.hpp"
#include <cmath>
#include <glm/detail/func_common.hpp>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <omp.h>

#ifdef _OPENMP
    #pragma message("OpenMP is ENABLED")
#else
    #pragma message("OpenMP is NOT enabled")
#endif


Terrain::Terrain(glm::vec3 _scale, int _resolution) {
    // Position of terrain is speacial, because it will need to match the camera
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = _scale;

    //
    
    fov = 45.0f;

    // Controls how "steep" the slopes are
    peakHeight = 800.0f;
    resolution = _resolution;
    octaves = 5;
    persistence = 0.503;
    lacunarity = 2;
    pn = PerlinNoise(-1);

    modeWireframe = false;

    consistencyFactor = resolution / scale.x;

    index_buffer_data = generate_grid_indices_acw(resolution);
    // Populate vertex_buffer_data to form a grid of 16 * 16
    // in a 1x1 area
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            float u0 = static_cast<float>(x) / (float)resolution - 0.5f;
            float v0 = static_cast<float>(z) / (float)resolution - 0.5f;
            
            float u = u0 + static_cast<float>(offset.x * consistencyFactor) / (float)resolution;
            float v = v0 + static_cast<float>(offset.z * consistencyFactor) / (float)resolution;
            
            float h =  pn.octavePerlin(u, v, octaves, persistence, lacunarity);
            // Map u.v to [-1, 1] * scale
            glm::vec3 coords = glm::vec3(
                scale.x * (u),
                peakHeight * h,
                scale.z * (v)
            );
            vertex_buffer_data.push_back(coords);
        }
    }

    // Generate face normals using index buffer information. Every 3 is 1 triangle
    // Then generate vertex normals by averaging face normals
    face_normals = std::vector<glm::vec3>(vertex_buffer_data.size(), glm::vec3(0, 0, 0));
    for (int i = 0; i < index_buffer_data.size(); i += 3) {
        glm::vec3 v0 = vertex_buffer_data[index_buffer_data[i]];
        glm::vec3 v1 = vertex_buffer_data[index_buffer_data[i + 1]];
        glm::vec3 v2 = vertex_buffer_data[index_buffer_data[i + 2]];

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        
        face_normals[index_buffer_data[i]] += normal;
        face_normals[index_buffer_data[i + 1]] += normal;
        face_normals[index_buffer_data[i + 2]] += normal;
    }

    for (int i = 0; i < vertex_buffer_data.size(); i++) {
        normal_buffer_data.push_back(glm::normalize(face_normals[i]));
    }
};

void Terrain::initialize(std::shared_ptr<Shader> shaderptr, glm::vec3 position) {

    this->position = position;
    this->scale = scale;

    glGenVertexArrays(1, &vertexArrayID);

    
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_buffer_data.size(), &vertex_buffer_data[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal_buffer_data.size(), &normal_buffer_data[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_buffer_data.size(), &index_buffer_data[0], GL_STATIC_DRAW);


    glBindVertexArray(0);

    
    this->shader = shaderptr;
    if (shaderptr->getProgramID() == 0) {
        std::cerr << "Invalid shader provided to Terrain::initialize()\n";
    }
}

void Terrain::setNoiseParams(int octaves, float persistence, float lacunarity) {
    this->octaves = octaves;
    this->persistence = persistence;
    this->lacunarity = lacunarity;
}

void Terrain::setPeakHeight(float peakHeight) {
    this->peakHeight = peakHeight;
}

float Terrain::getCenterHeight() {
    return vertex_buffer_data[(resolution / 2) + (resolution / 2) * resolution].y;
}

bool Terrain::groundHeightConstraint(glm::vec3 &position) {
    // Always be 50 units above the ground
    if (position.y < getCenterHeight() + 50) {
        position.y = getCenterHeight() + 50;
        return true;
    }
    return false;
}

void Terrain::setWireframeMode(bool enabled) {
    modeWireframe = enabled;
}

void Terrain::render(glm::mat4 vp, const LightingParams& lightingParams) {
    glm::mat4 modelMatrix = glm::mat4();
    modelMatrix = glm::translate(modelMatrix, position);
    // Special scale factor:
    modelMatrix = glm::scale(modelMatrix, specialScale);
    
    glm::mat4 mvp = vp * modelMatrix;

    shader->use();
    shader->setUniMat4("MVP", mvp);
    shader->setUniMat4("Model", modelMatrix);
    shader->setUniVec3("lightPosition", lightingParams.lightPosition);
    glBindVertexArray(vertexArrayID);

    // Wireframe
    if (modeWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glDrawElements(GL_TRIANGLES, index_buffer_data.size(), GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);

};

void Terrain::update(float deltaTime){

    float heightFromGround = offset.y;
    const float originalHeight = 300.0f; //! TODO: UPDATE THIS IF WE CHANGE CAMERA
    specialScale =  glm::max(2.0f, heightFromGround / originalHeight) *  glm::vec3(1, 0, 1) + glm::vec3(0, 1, 0);

    // Update perlin noise height
    float offsetU = offset.x * consistencyFactor;
    float offsetV = offset.z * consistencyFactor;

    #pragma omp parallel for
    for (int z = 0; z < resolution; z++) {
        float v0 = specialScale.z * (static_cast<float>(z) / (float)resolution - 0.5);
        #pragma omp parallel for
        for (int x = 0; x < resolution; x++) {
            float u0 = specialScale.x * (static_cast<float>(x) / (float)resolution - 0.5);

            float u = u0 + static_cast<float>(offsetU) / (float)resolution;
            float v = v0 + static_cast<float>(offsetV) / (float)resolution;

            float h = pn.octavePerlin(u, v, octaves, persistence, lacunarity);

            vertex_buffer_data[x + z * resolution].y = peakHeight * (h - 0.5) * 2;
        }
    }


    // - For averaging face normals
    // std::fill(face_normals.begin(), face_normals.end(), glm::vec3(0.0f));

    // #pragma omp parallel for
    // for (int i = 0; i < index_buffer_data.size(); i += 3) {
    //     glm::vec3 v0 = vertex_buffer_data[index_buffer_data[i]];
    //     glm::vec3 v1 = vertex_buffer_data[index_buffer_data[i + 1]];
    //     glm::vec3 v2 = vertex_buffer_data[index_buffer_data[i + 2]];

    //     glm::vec3 normal = (glm::cross(v1 - v0, v2 - v0));
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i]].x += normal.x;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i]].y += normal.y;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i]].z += normal.z;

    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 1]].x += normal.x;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 1]].y += normal.y;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 1]].z += normal.z;

    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 2]].x += normal.x;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 2]].y += normal.y;
    //     #pragma omp atomic
    //     face_normals[index_buffer_data[i + 2]].z += normal.z;
    // }
    // #pragma omp parallel for
    // for (int i = 0; i < vertex_buffer_data.size(); i++) {
    //     normal_buffer_data[i] = face_normals[i];
    // }
    
    // - Faster but less accurate
    #pragma omp parallel for
    for (int i = 0; i < vertex_buffer_data.size(); i++) {
        // Find the first face that this vertex is part of
        // We know that vertex i:
        // if i % resolution <= resolution - 2 and 
        // if i // resolution <= resolution - 2 
        // is in i, i + resolution, i + 1
        int row = i / resolution;
        int col = i % resolution;
        if (col <= (resolution - 2) && row <= (resolution - 2)) {
            glm::vec3 v0 = vertex_buffer_data[i];
            glm::vec3 v1 = vertex_buffer_data[i + resolution];
            glm::vec3 v2 = vertex_buffer_data[i + 1];
            normal_buffer_data[i] = glm::cross(v1 - v0, v2 - v0);
        }
    }

    glBindVertexArray(vertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_buffer_data.size(), &vertex_buffer_data[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal_buffer_data.size(), &normal_buffer_data[0], GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

};

/**
 * @brief 
 * Call this after we have created a shader for it
 * @param shaderProgramID 
 */
void Terrain::updateOffset(glm::vec3 newOffset) {
    position = glm::vec3(newOffset.x, 0, newOffset.z);
    offset = newOffset;
}
