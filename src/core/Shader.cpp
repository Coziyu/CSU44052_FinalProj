#include "Shader.hpp"

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource){
    id = LoadShadersFromFile(vertexShaderSource, fragmentShaderSource);
}

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource, const char* geometryShaderSource)
{ 
    id = LoadShadersFromFile(vertexShaderSource, fragmentShaderSource, geometryShaderSource);
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

void Shader::use(){
    glUseProgram(id);
}

void Shader::setUniBool(const std::string &name, bool value){
    glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
}

void Shader::setUniInt(const std::string &name, int value){
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setUniFloat(const std::string &name, float value){
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setUniVec2(const std::string &name, const glm::vec2 &value){
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setUniVec3(const std::string &name, const glm::vec3 &value){
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setUniVec4(const std::string &name, const glm::vec4 &value){
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setUniVec2(const std::string &name, float x, float y){
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::setUniVec3(const std::string &name, float x, float y, float z){
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::setUniVec4(const std::string &name, float x, float y, float z, float w){
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::setUniMat2(const std::string &name, const glm::mat2 &mat){
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniMat3(const std::string &name, const glm::mat3 &mat){
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniMat4(const std::string &name, const glm::mat4 &mat){
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniMat4Arr(const std::string &name, const std::vector<glm::mat4> &mat, const int size){
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), size, GL_FALSE, &mat[0][0][0]);
}

GLuint Shader::getProgramID() const { return id; }
