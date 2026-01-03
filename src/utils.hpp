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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

std::vector<unsigned int> generate_grid_indices_acw(const unsigned int size);
std::vector<unsigned int> generate_grid_indices_cw(const unsigned int size);

GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path);
GLuint LoadShadersFromFile(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path);

std::string readFileAsString(const char* filename);

struct AxisAngle {
    glm::vec3 axis;
    float angle;
};

AxisAngle rotationFromTo(glm::vec3 from, glm::vec3 to);
glm::quat rotationFromToQuat(glm::vec3 from, glm::vec3 to);

glm::quat myQuatLookAt(glm::vec3 direction, glm::vec3 up);

#endif // UTILS_HPP
