#include "Mesh.hpp"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures) {
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	VAO.bind();
	VBO VBO(vertices);
	EBO EBO(indices);
	VAO.setAttribPtr(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.setAttribPtr(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.setAttribPtr(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.setAttribPtr(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	VAO.unbind();
	VBO.unbind();
	EBO.unbind();
}


void Mesh::Draw(Shader& shader, Camera& camera) {
	// Bind shader to be able to access uniforms
	shader.use();
	VAO.bind();

	// Keep track of how many of each type of textures we have
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		textures[i].setTexUnit(shader, (type + num).c_str(), i);
		textures[i].bind();
	}
	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.getProgramID(), "camPos"), camera.position.x, camera.position.y, camera.position.z);
	shader.setUniMat4("VP", camera.getProjectionMatrix() * camera.getViewMatrix());

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}