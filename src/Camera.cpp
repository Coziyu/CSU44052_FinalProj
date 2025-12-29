#include "Camera.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(
            glm::vec3 startPosition, 
            glm::vec3 up, 
            float startYaw,
            float startPitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), 
    movementSpeed(DEFAULT::SPEED),
    mouseSensitivity(DEFAULT::SENSITIVITY)
{
    position = startPosition;
    worldUp = up;
    yaw = startYaw;
    pitch = startPitch;
    updateCameraVectors();
};

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, top);
}

void Camera::handleKeyboardInput(DIRECTIONS direction, float deltaTime) {
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

void Camera::handleMouseInput(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    // Ensure screen doesn't flip
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

    front      = glm::normalize(_newfront);
    right      = glm::normalize(glm::cross(front, worldUp));
    top      = glm::normalize(glm::cross(right, front));
}
