#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

Camera::Camera(
            glm::vec3 startPosition, 
            glm::vec3 up, 
            glm::vec3 startFront
) : movementSpeed(DEFAULT::SPEED),
    mouseSensitivity(DEFAULT::SENSITIVITY)
{
    // Convert startFront to yaw and pitch
    front = glm::normalize(startFront);
    yaw = glm::degrees(atan2(front.z, front.x));
    pitch = glm::degrees(asin(front.y));

    position = startPosition;
    worldUp = up;
    front = startFront;
    updateCameraVectors();
};

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, top);
}

void Camera::processInput(Window& window, const float deltaTime) {
    if (window.isKeyPressed(GLFW_KEY_W))
        this->handleMovement(FORWARD, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_S))
        this->handleMovement(BACKWARD, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_A))
        this->handleMovement(LEFT, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_D))
        this->handleMovement(RIGHT, deltaTime);

    if (window.isKeyPressed(GLFW_KEY_TAB)) {
        std::cout << "Camera Position: (" 
                  << position.x << ", " 
                  << position.y << ", " 
                  << position.z << ")\n";
        std::cout << "Camera yaw: " << yaw << ", pitch: " << pitch << "\n";
        std::cout << "Camera front: (" 
                << front.x << ", " 
                << front.y << ", " 
                << front.z << ")\n";
        std::cout << "Camera right: (" 
                << right.x << ", " 
                << right.y << ", " 
                << right.z << ")\n";
    }
}

void Camera::handleMovement(DIRECTIONS direction, const float deltaTime) {
    float speed = movementSpeed * deltaTime;
    if (direction == FORWARD)
        position += front * speed;
    if (direction == BACKWARD)
        position -= front * speed;
    if (direction == LEFT)
        position -= right * speed;
    if (direction == RIGHT)
        position += right * speed;
}

void Camera::handleMouseInput(double xOffset, double yOffset) {
    static bool firstMouse = true;
    if (firstMouse) {
        lastMouseX = xOffset;
        lastMouseY = yOffset;
        firstMouse = false;
    }
    double dx = xOffset - lastMouseX;
    double dy = lastMouseY - yOffset; // Reversed since y-coordinates go from bottom to top

    lastMouseX = xOffset;
    lastMouseY = yOffset;

    dx *= mouseSensitivity;
    dy *= mouseSensitivity;

    yaw   += dx;
    pitch += dy;

    // Ensure screen doesn't flip
    bool constrainPitch = true;
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 _newfront;
    _newfront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    _newfront.y = sin(glm::radians(pitch));
    _newfront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(_newfront);
    right = glm::normalize(glm::cross(front, worldUp));
    top   = glm::normalize(glm::cross(right, front));
}

// Note: We register callbacks for methods that changes state only.
// Otherwise, we simply use polling methods in main loop.


