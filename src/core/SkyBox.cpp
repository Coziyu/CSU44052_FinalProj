#include "SkyBox.hpp"
#include "utils.hpp"
#include <glm/detail/type_vec.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stb/stb_image.h>

void SkyBox::initialize(glm::vec3 position, glm::vec3 scale) {
    this->position = position;
    this->scale = scale;
    
    // texel insetting idea from:
    // https://gamedev.stackexchange.com/questions/30062/problem-with-drawing-textures-in-opengl-es
    float texelInset = 0.0015;
    for (int face = 0; face < 6; face++) {
        int base = face * 8;
        
        float minU = uv_buffer_data[base];
        float maxU = uv_buffer_data[base];
        float minV = uv_buffer_data[base + 1];
        float maxV = uv_buffer_data[base + 1];
        
        // find edges
        for (int v = 1; v < 4; v++) {
            float u = uv_buffer_data[base + v * 2];
            float vCoord = uv_buffer_data[base + v * 2 + 1];
            minU = std::min(minU, u);
            maxU = std::max(maxU, u);
            minV = std::min(minV, vCoord);
            maxV = std::max(maxV, vCoord);
        }
        
        // inset each uv towards the tile center
        for (int v = 0; v < 4; v++) {
            float& u = uv_buffer_data[base + v * 2];
            float& vCoord = uv_buffer_data[base + v * 2 + 1];
            
            if (u == minU) u += texelInset;
            else if (u == maxU) u -= texelInset;
            
            if (vCoord == minV) vCoord += texelInset;
            else if (vCoord == maxV) vCoord -= texelInset;
            
            u /= 4.0f;
            vCoord = 1.0f - (vCoord / 3.0f);  // inverted because png is loaded upside down
        }
    }



    // Create a vertex array object
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Create a vertex buffer object to store the vertex data
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );
    glEnableVertexAttribArray(0);

    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    // Create and compile our GLSL program from the shaders
    programID = LoadShadersFromFile("../shaders/skybox.vert", "../shaders/skybox.frag");
    if (programID == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }
    // Get a handle for our "MVP" uniform
    mvpMatrixID = glGetUniformLocation(programID, "MVP");

    // Load texture
    // std::string texture_file_path = "../assets/scifi_skybox.png";
    std::string texture_file_path = "../assets/skybox.png";

    textureID = LoadTextureSkyBox(texture_file_path.c_str());

    textureSamplerID = glGetUniformLocation(programID, "textureSampler");

    glBindVertexArray(0);
}

void SkyBox::render(glm::mat4 cameraMatrix) {
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Model transform
    glm::mat4 modelMatrix = glm::mat4();
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    
    glm::mat4 mvp = cameraMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


    // Set textureSampler to use texture unit 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 2);

    // Draw the box
    glDrawElements(
        GL_TRIANGLES,      // mode
        36,    			   // number of indices
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    glBindVertexArray(0);
}


void SkyBox::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteTextures(1, &textureID);
    glDeleteProgram(programID);
}

void SkyBox::update(glm::vec3 position) {
	// keep skybox centered on camera
    this->position = position;
}

GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}

GLuint LoadTextureSkyBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}