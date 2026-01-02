#ifndef EBO_HPP
#define EBO_HPP

#include <glad/gl.h>
#include <vector>

class EBO {
	public:
		GLuint id;
		EBO(std::vector<GLuint>& indices);

		void bind();
		void unbind();
		void cleanup();
};


#endif // EBO_HPP
