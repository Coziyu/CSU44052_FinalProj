#include <iostream>
#include "Camera.hpp"
#include "Window.hpp"
#include <glm/gtc//matrix_transform.hpp>


#include "Cube.hpp"




int main() {
    Window window(1366, 768, "Wonderland");
    window.initialize();
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

    Box testBox;
    testBox.initialize(glm::vec3(0, 100, 0), glm::vec3(10, 10, 10));
    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    do {
        float currTime = (glfwGetTime());
        deltaTime = currTime - lastTime;
        lastTime = currTime;
        
        camera.processInput(window, deltaTime);
        window.recomputeFPS(deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            static_cast<float>(window.width) / static_cast<float>(window.height), 
            0.1f, 
            1000.0f
        );
        testBox.render(projection * view);

        window.swapBuffers();
        window.pollEvents();
    } 
    while (!window.shouldClose());

    std::cout << "Hello, World!" << std::endl;
    return 0;
}



