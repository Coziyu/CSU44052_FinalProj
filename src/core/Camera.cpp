#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <imgui.h>
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

    flightMode = false;

    updateCameraVectors();
};

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, top);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

void Camera::processInput(Window& window, const float deltaTime) {
    static int prevCapsState = GLFW_RELEASE;

    float simDeltaTime = deltaTime;
    if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        simDeltaTime *= 10.0f;
    }
    if (window.isKeyPressed(GLFW_KEY_W))
        this->handleMovement(FORWARD, simDeltaTime);
    if (window.isKeyPressed(GLFW_KEY_S))
        this->handleMovement(BACKWARD, simDeltaTime);
    if (window.isKeyPressed(GLFW_KEY_A))
        this->handleMovement(LEFT, simDeltaTime);
    if (window.isKeyPressed(GLFW_KEY_D))
        this->handleMovement(RIGHT, simDeltaTime);
    if (window.isKeyPressed(GLFW_KEY_SPACE))
        this->handleMovement(UP, simDeltaTime);
    if (window.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
        this->handleMovement(DOWN, simDeltaTime);
    
    if (window.isKeyReleased(GLFW_KEY_CAPS_LOCK) && prevCapsState == GLFW_PRESS){
        // Toggle flight mode
        flightMode = !flightMode;
        std::cout << "Flight mode: " << (flightMode ? "ON" : "OFF") << "\n";
    }
    prevCapsState = window.isKeyPressed(GLFW_KEY_CAPS_LOCK);
    

    if (window.isKeyPressed(GLFW_KEY_APOSTROPHE)) {
        std::cout << "Camera Position: (" 
                  << position.x << ", " 
                  << position.y << ", " 
                  << position.z << ")\n";
    }
}

void Camera::update(const float deltaTime) {
    // If flight mode not enabled, physics update
    if (!flightMode) {
        fallSpeed += DEFAULT::GRAVITY * deltaTime;
        position.y += fallSpeed * deltaTime;
    }
    else {
        resetFallSpeed();
    }
}

void Camera::resetFallSpeed() {
    fallSpeed = 0;
}

void Camera::setOnGround(bool onGround) { 
    this->onGround = onGround;
}

void Camera::setFov(float fov) { this->fov = fov; }

void Camera::setZNear(float zNear) { this->zNear = zNear; }

void Camera::setZFar(float zFar) { this->zFar = zFar; }

void Camera::setAspect(float aspect) { this->aspect = aspect; }

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
    if (direction == UP) {
        if (!flightMode) {
            if (onGround)
                fallSpeed = - 0.5 * DEFAULT::GRAVITY;
        }
        else {
            position += worldUp * 2.0f * speed;
        }
    }
    if (direction == DOWN)
        position -= worldUp * speed;
}

void Camera::handleMouseInput(double newMouseX, double newMouseY, bool cursorLocked) {
    static bool firstMouse = true;
    if (!cursorLocked) {
        // Ensure no snapping when cursor is unlocked
        lastMouseX = newMouseX;
        lastMouseY = newMouseY;
        return;
    }
    if (firstMouse) {
        lastMouseX = newMouseX;
        lastMouseY = newMouseY;
        firstMouse = false;
    }
    double dx = newMouseX - lastMouseX;
    double dy = lastMouseY - newMouseY; // Reversed since y-coordinates go from bottom to top

    lastMouseX = newMouseX;
    lastMouseY = newMouseY;

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


