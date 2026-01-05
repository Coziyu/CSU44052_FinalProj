#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

/**
 * @brief Interface representing a drawable entity.
 */
#include <glm/detail/type_mat.hpp>
#include <glm/fwd.hpp>
#include "LightingParams.hpp"

class Drawable {
    public:
        virtual void render(glm::mat4 vp, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane = 10000.0f) = 0;        
};

#endif // DRAWABLE_HPP
