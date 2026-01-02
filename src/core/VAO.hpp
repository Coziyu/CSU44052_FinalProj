#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include "VBO.hpp"

class VAO {
	public:
		GLuint id;
		VAO();

		// set the wbo Attribute in the vao
		void setAttribPtr(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
		void bind();
		void unbind();
		void cleanup();
};

#endif