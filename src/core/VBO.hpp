#ifndef VBO_HPP
#define VBO_HPP

#include <glm/glm.hpp>
#include <glad/gl.h>

#include <vector>

// Structure to standardize the vertices used in the meshes
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};



class VBO {
	public:
		GLuint id;
		VBO(std::vector<Vertex>& vertices);

		void bind();
		void unbind();
		void cleanup();
};


#endif // VBO_HPP
