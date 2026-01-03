#ifndef PHOENIX_HPP
#define PHOENIX_HPP

#include "ModelEntity.hpp"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct Phoenix : public ModelEntity {
    static std::string modelDirectory;
    static std::string modelPath;
    static std::string vertexShaderPath;
    static std::string fragmentShaderPath;
	Phoenix() : ModelEntity() {};
    void initialize(bool isSkinned);
    void update(float dt);
}; 

#endif // PHOENIX_HPP
