#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glm/glm.hpp"

// Implementation based off of LearnOpenGL.com's Camera class

/**
 * @brief Class representing a camera in the 3D scene.
 * This will be controllable by the user to navigate the scene.
 */

enum DIRECTIONS {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


// Default camera values
namespace DEFAULT {
    const float YAW         = -90.0f;
    const float PITCH       =  0.0f;
    const float SPEED       =  2.5f;
    const float SENSITIVITY =  0.1f;
}

class Camera {
    public:
        // camera attributes
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 top;
        glm::vec3 right;
        glm::vec3 worldUp;

        // euler angles
        float yaw;
        float pitch;

        // camera options
        float movementSpeed;
        float mouseSensitivity;

        /**
         * @brief Construct a new Camera object
         * 
         * @param startPosition starting position of the camera
         * @param up world up vector
         */
        Camera(
            glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
            float startYaw = DEFAULT::YAW,
            float startPitch = DEFAULT::PITCH
        );
        glm::mat4 getViewMatrix() const;
        
        /**
        * @brief Handles keyboard input to move the camera.
        * This is to be added as a callback to the window
        */
        void handleKeyboardInput(DIRECTIONS direction, float deltaTime);
        /**
        * @brief Handles mouse movement to rotate the camera.
        * This is to be added as a callback to the window
        */
        void handleMouseInput(float xOffset, float yOffset, bool constrainPitch = true);

    private:
        /**
         * @brief Updates the camera's front, right, and top vectors based on the current yaw and pitch.
         * This is to be whenever the camera's parameters mutate.
         */
        void updateCameraVectors();
};

#endif // CAMERA_HPP
