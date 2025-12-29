#include <iostream>
#include "Camera.hpp"
#include "Window.hpp"
#include <glm/gtc//matrix_transform.hpp>


int main() {
    
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

void processInput(Window& window, Camera& camera, float deltaTime) {
    if (window.isKeyPressed(GLFW_KEY_W))
        camera.handleKeyboardInput(FORWARD, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_S))
        camera.handleKeyboardInput(BACKWARD, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_A))
        camera.handleKeyboardInput(LEFT, deltaTime);
    if (window.isKeyPressed(GLFW_KEY_D))
        camera.handleKeyboardInput(RIGHT, deltaTime);
}

void renderFromCameraToScreen(const Camera& camera, const Window& window) {
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), 
        static_cast<float>(window.width) / static_cast<float>(window.height), 
        0.1f, 
        1000.0f
    );

    // TODO: Remove this placeholder, and move this into a scene manager class
    // Just render a simple cube for now
    
}

