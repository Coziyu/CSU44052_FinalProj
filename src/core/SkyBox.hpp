#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "glm/detail/type_vec.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

struct SkyBox {
	glm::vec3 position;		// Position of the box 
	glm::vec3 scale;		// Size of the box in each axis
	
	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f, 
		1.0f, -1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		-1.0f, 1.0f, 1.0f, 
		
		// Back face 
		1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f, -1.0f, 
		-1.0f, 1.0f, -1.0f, 
		1.0f, 1.0f, -1.0f,
		
		// Left face
		-1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f, 1.0f, 
		-1.0f, 1.0f, 1.0f, 
		-1.0f, 1.0f, -1.0f, 

		// Right face 
		1.0f, -1.0f, 1.0f, 
		1.0f, -1.0f, -1.0f, 
		1.0f, 1.0f, -1.0f, 
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, -1.0f, 
		-1.0f, 1.0f, -1.0f, 

		// Bottom face
		-1.0f, -1.0f, -1.0f, 
		1.0f, -1.0f, -1.0f, 
		1.0f, -1.0f, 1.0f, 
		-1.0f, -1.0f, 1.0f, 
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		2, 1, 0, 	
		3, 2, 0, 
		
		6, 5, 4, 
		7, 6, 4, 

		10, 9, 8, 
		11, 10, 8, 

		14, 13, 12, 
		15, 14, 12, 

		18, 17, 16, 
		19, 18, 16, 

		22, 21, 20, 
		23, 22, 20, 
	};

    // Done: Define UV buffer data
    // ---------------------------
	GLfloat uv_buffer_data[48] = {
		// Front +Z
		2.0f, 2.0f,
		1.0f, 2.0f,
		1.0f, 1.0f,
		2.0f, 1.0f,

		// Back -Z
		4.0f, 2.0f,
		3.0f, 2.0f,
		3.0f, 1.0f,
		4.0f, 1.0f,

		// Left +X
		3.0f, 2.0f,
		2.0f, 2.0f,
		2.0f, 1.0f,
		3.0f, 1.0f,
        
		// Right -X
		1.0f, 2.0f,
		0.0f, 2.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		// Bottom -Y
		2.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		2.0f, 0.0f,

		// Top +Y
		2.0f, 3.0f,
		1.0f, 3.0f,
		1.0f, 2.0f,
		2.0f, 2.0f,
	};
    // ---------------------------
    
	// OpenGL buffers
	GLuint vertexArrayID; 
	GLuint vertexBufferID; 
	GLuint indexBufferID; 
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint textureSamplerID;
	GLuint mvpMatrixID;
	GLuint programID;
	GLuint timeID;

	void initialize(glm::vec3 position, glm::vec3 scale);

	void render(glm::mat4 cameraMatrix);

	void cleanup();

	void update(glm::vec3 position);

}; 

GLuint LoadTextureSkyBox(const char *texture_file_path);

GLuint LoadTextureTileBox(const char *texture_file_path);

#endif // SKYBOX_HPP
