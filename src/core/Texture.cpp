#include "Texture.hpp"
#include <stb/stb_image.h>

Texture::Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType){
	// Assigns the type of the texture ot the texture object
	type = texType;

	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(GL_TEXTURE_2D, ID);

	// TODO: Set this for different types of textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setTexUnit(Shader& shader, const char* uniform, GLuint unit) {
	shader.setUniInt(uniform, unit);
}

void Texture::bind() {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::cleanup() {
	glDeleteTextures(1, &ID);
}