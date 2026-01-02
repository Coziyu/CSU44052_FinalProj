
#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Shader.hpp"

class Texture{
	public:
		GLuint ID;
		const char* type;
		GLuint unit;

		Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType);

		// Assigns a texture unit to a texture
		void setTexUnit(Shader& shader, const char* uniform, GLuint unit);

		void bind();
		void unbind();
		void cleanup();

};

#endif // TEXTURE_HPP
