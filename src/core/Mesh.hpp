#ifndef MESH_HPP
#define MESH_HPP

#include <string>

#include "VAO.hpp"
#include "EBO.hpp"
#include "Texture.hpp"
#include "Camera.hpp"

class Mesh {
	public:
		std::vector <Vertex> vertices;
		std::vector <GLuint> indices;
		std::vector <Texture> textures;
		// Store VAO in public so it can be used in the Draw function
		VAO VAO;

		// Initializes the mesh
		Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);

		// Draws the mesh
		void Draw(Shader& shader, Camera& camera);
};


#endif // MESH_HPP
