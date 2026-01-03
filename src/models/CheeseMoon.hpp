#ifndef CHEESEMOON_HPP
#define CHEESEMOON_HPP

#include "ModelEntity.hpp"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct CheeseMoon : public ModelEntity {
    static std::string modelDirectory;
    static std::string modelPath;
    static std::string vertexShaderPath;
    static std::string fragmentShaderPath;
	CheeseMoon() : ModelEntity() {};
    void initialize(bool isSkinned);
    void update(float dt, glm::vec3 cameraPos);
}; 

#endif // CHEESEMOON_HPP
