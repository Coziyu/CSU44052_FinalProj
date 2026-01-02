#include "VAO.hpp"

VAO::VAO() {
	glGenVertexArrays(1, &id);
}

void VAO::setAttribPtr(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
	VBO.bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.unbind();
}

void VAO::bind() {
	glBindVertexArray(id);
}

void VAO::unbind() {
	glBindVertexArray(0);
}

void VAO::cleanup() {
	glDeleteVertexArrays(1, &id);
}