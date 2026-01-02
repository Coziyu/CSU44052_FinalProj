#ifndef UPDATEABLE_HPP
#define UPDATEABLE_HPP

/**
 * @brief Interface for updateable entities.
 */
class Updateable {
    public:
        virtual void update(float deltaTime) = 0;        
};

#endif // UPDATEABLE_HPP
