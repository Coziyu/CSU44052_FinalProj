#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glm/glm.hpp"
#include "Window.hpp"

// Implementation based off of LearnOpenGL.com's Camera class

/**
 * @brief Class representing a camera in the 3D scene.
 * This will be controllable by the user to navigate the scene.
 */

enum DIRECTIONS {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};


// Default camera values
namespace DEFAULT {
    const float YAW         =  0.0f;
    const float PITCH       =  0.0f;
    const float SPEED       =  200.0f;
    const float SENSITIVITY =  0.1f;
    const float GRAVITY     = -981.0f;
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

        // Projection params
        float fov;
        float zNear;
        float zFar;
        float aspect;

        // mouse
        double lastMouseX;
        double lastMouseY;

        // State
        bool flightMode;
        float fallSpeed;
        bool onGround;

        /**
         * @brief Construct a new Camera object
         * 
         * @param startPosition starting position of the camera
         * @param up world up vector
         */
        Camera(
            glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
            glm::vec3 startFront = glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix() const;
        /**
        * @brief Handles keyboard input to move the camera.
        * This is to be added as a callback to the window
        */
        void handleMovement(DIRECTIONS direction, const float deltaTime);
        /**
        * @brief Handles mouse movement to rotate the camera.
        * This is to be added as a callback to the window
        */
        void handleMouseInput(double xOffset, double yOffset, bool cursorLocked);

        void processInput(Window& window, const float deltaTime);

        void update(const float deltaTime);

        void resetFallSpeed();

        void setOnGround(bool onGround);

        void setFov(float fov);
        void setZNear(float zNear);
        void setZFar(float zFar);
        void setAspect(float aspect);

        glm::vec3 getPosition() const;

      private:
        /**
         * @brief Updates the camera's front, right, and top vectors based on the current yaw and pitch.
         * This is to be whenever the camera's parameters mutate.
         */
        void updateCameraVectors();

        
        // TODO: Note the use of when vs on prefix
        // The former is for event handling, the latter is for state updating
};

#endif // CAMERA_HPP
