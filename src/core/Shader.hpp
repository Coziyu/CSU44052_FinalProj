#ifndef SHADER_HPP
#define SHADER_HPP
#include "utils.hpp"
#include <string>

/**
 * @brief Class representing a shader program.
 */

class Shader {
    private:
    GLuint id;
    
    public:
    Shader(const char* vertexShaderSource, const char* fragmentShaderSource);
    Shader(const char* vertexShaderSource, const char* fragmentShaderSource, const char* geometryShaderSource);
    ~Shader();
    GLuint getProgramID() const;

    void use();

    void setUniBool(const std::string &name, bool value);
    void setUniInt(const std::string &name, int value);
    void setUniFloat(const std::string &name, float value);
    void setUniVec2(const std::string &name, const glm::vec2 &value);
    void setUniVec3(const std::string &name, const glm::vec3 &value);
    void setUniVec4(const std::string &name, const glm::vec4 &value);
    void setUniVec2(const std::string &name, float x, float y);
    void setUniVec3(const std::string &name, float x, float y, float z);
    void setUniVec4(const std::string &name, float x, float y, float z, float w);
    void setUniMat2(const std::string &name, const glm::mat2 &mat);
    void setUniMat3(const std::string &name, const glm::mat3 &mat);
    void setUniMat4(const std::string &name, const glm::mat4 &mat);

    void setUniMat4Arr(const std::string &name, const std::vector<glm::mat4> &mat, const int size);

};

#endif // SHADER_HPP
