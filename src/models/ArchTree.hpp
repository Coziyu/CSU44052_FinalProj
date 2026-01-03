#ifndef ARCHTREE_HPP
#define ARCHTREE_HPP

#include "ModelEntity.hpp"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct ArchTree : public ModelEntity {
    static std::string modelDirectory;
    static std::string modelPath;
    static std::string vertexShaderPath;
    static std::string fragmentShaderPath;
	ArchTree() : ModelEntity() {};
    void initialize(bool isSkinned);
}; 

#endif // ARCHTREE_HPP
