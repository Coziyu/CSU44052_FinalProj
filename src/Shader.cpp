#include "Shader.hpp"

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource){
    programID = LoadShadersFromFile(vertexShaderSource, fragmentShaderSource);
}

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource, const char* geometryShaderSource)
{ 
    programID = LoadShadersFromFile(vertexShaderSource, fragmentShaderSource, geometryShaderSource);
}

Shader::~Shader()
{
    glDeleteProgram(programID);
}

void Shader::use(){
    glUseProgram(programID);
}

void Shader::setUniBool(const std::string &name, bool value){
    glUniform1i(glGetUniformLocation(programID, name.c_str()), static_cast<int>(value));
}

void Shader::setUniInt(const std::string &name, int value){
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setUniFloat(const std::string &name, float value){
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setUniVec2(const std::string &name, const glm::vec2 &value){
    glUniform2f(glGetUniformLocation(programID, name.c_str()), value.x, value.y);
}

void Shader::setUniVec3(const std::string &name, const glm::vec3 &value){
    glUniform3f(glGetUniformLocation(programID, name.c_str()), value.x, value.y, value.z);
}

void Shader::setUniVec4(const std::string &name, const glm::vec4 &value){
    glUniform4f(glGetUniformLocation(programID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setUniVec2(const std::string &name, float x, float y){
    glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}

void Shader::setUniVec3(const std::string &name, float x, float y, float z){
    glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}

void Shader::setUniVec4(const std::string &name, float x, float y, float z, float w){
    glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
}

void Shader::setUniMat2(const std::string &name, const glm::mat2 &mat){
    glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniMat3(const std::string &name, const glm::mat3 &mat){
    glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniMat4(const std::string &name, const glm::mat4 &mat){
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
GLuint Shader::getProgramID() const { return programID; }
