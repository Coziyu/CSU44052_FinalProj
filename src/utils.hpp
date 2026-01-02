#ifndef UTILS_HPP
#define UTILS_HPP
#ifndef GL_H
#define GL_H
#include <glad/gl.h>
#include <glfw/glfw3.h>
#endif 
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<unsigned int> generate_grid_indices_acw(const unsigned int size);
std::vector<unsigned int> generate_grid_indices_cw(const unsigned int size);

GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path);
GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path);

std::string readFileAsString(const char* filename);

#endif // UTILS_HPP
