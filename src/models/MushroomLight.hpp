#ifndef MUSHROOMLIGHT_HPP
#define MUSHROOMLIGHT_HPP

#include "ModelEntity.hpp"

struct MushroomLight : public ModelEntity {
    static std::string modelDirectory;
    static std::string modelPath;
    static std::string vertexShaderPath;
    static std::string fragmentShaderPath;
	MushroomLight() : ModelEntity() {};
    void initialize(bool isSkinned);
    void update(float dt);
}; 

#endif // MUSHROOMLIGHT_HPP
