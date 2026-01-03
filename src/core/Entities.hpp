#ifndef ENTITIES_HPP
#define ENTITIES_HPP

#include "Drawable.hpp"
#include "Shader.hpp"
#include "Updateable.hpp"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <glfw/glfw3.h>
#include <memory>

class Entity : public Drawable {
    public:
        int getEntityID() const;
        void setEntityID(int id);

        Entity();

        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotationAxis;
        float rotationAngle;
        glm::vec3 getPosition() const;
        void setPosition(const glm::vec3 &position_);
        void setScale(const glm::vec3 &scale_);
        glm::vec3 getScale() const;
    protected:
        int entityID;
        std::shared_ptr<Shader> shader;

        
};

class DynamicEntity : public Entity, public Updateable {
    public:
        virtual void update(float deltaTime) = 0;
};


#endif // ENTITIES_HPP
