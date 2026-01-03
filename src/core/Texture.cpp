#include "Texture.hpp"
#include <stb/stb_image.h>

Texture::Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType){
	// Assigns the type of the texture ot the texture object
	type = texType;

	int widthImg, heightImg, numColCh;
	// glTF stores images with top-left origin; do NOT flip vertically when loading for OpenGL
	stbi_set_flip_vertically_on_load(false);
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);
	bool allocated = false;
	if (!bytes) {
		std::cerr << "Failed to load texture: " << image << ". Creating 1x1 white fallback." << std::endl;
		widthImg = heightImg = 1;
		numColCh = 4;
		bytes = (unsigned char*)malloc(4);
		bytes[0] = 255; bytes[1] = 255; bytes[2] = 255; bytes[3] = 255;
		allocated = true;
	}

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(GL_TEXTURE_2D, ID);

	// TODO: Set this for different types of textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Choose proper formats based on channels
	GLenum dataFormat = GL_RGBA;
	GLenum internalFormat = GL_RGBA8;
	if (numColCh == 1) { dataFormat = GL_RED; internalFormat = GL_R8; }
	else if (numColCh == 3) { dataFormat = GL_RGB; internalFormat = GL_RGB8; }
	else if (numColCh == 4) { dataFormat = GL_RGBA; internalFormat = GL_RGBA8; }

	// Fix alignment for 3-channel images
	if (dataFormat == GL_RGB) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	else glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, widthImg, heightImg, 0, dataFormat, GL_UNSIGNED_BYTE, bytes);

	// Only generate mipmaps if texture upload succeeded
	glGenerateMipmap(GL_TEXTURE_2D);

	if (allocated) free(bytes);
	else stbi_image_free(bytes);

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