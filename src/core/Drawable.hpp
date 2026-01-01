#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

/**
 * @brief Interface representing a drawable entity.
 */
#include <glm/detail/type_mat.hpp>
class Drawable {
    public:
        virtual void render(glm::mat4 vp) = 0;        
};

#endif // DRAWABLE_HPP
