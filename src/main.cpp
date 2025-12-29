#include <iostream>
#include "Camera.hpp"
#include "Window.hpp"
#include <glm/gtc//matrix_transform.hpp>


#include "Cube.hpp"

int main() {
    Camera camera(
        glm::vec3(300.0f, 300.0f, 300.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f), 
        glm::vec3(0.0f, 100.0f, 0.0f) - glm::vec3(300.0f, 300.0f, 300.0f)
    );
    
    Window window(1366, 768, "Wonderland");
    window.initialize();
    
    window.registerMouseCallback([&camera](double x, double y) {
        camera.handleMouseInput(x, y);
    });

	// A coordinate system 
    AxisXYZ debugAxes;
    debugAxes.initialize();

	// A default box
	Box mybox;
	mybox.initialize(
        glm::vec3(0, 100, 0),  // translation
        glm::vec3(30, 30, 30)  // scale
    );
    


    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    do {
        float currTime = (glfwGetTime());
        deltaTime = currTime - lastTime;
        lastTime = currTime;
        
        camera.processInput(window, deltaTime);
        window.processInput();
        window.recomputeFPS(deltaTime);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            static_cast<float>(window.width) / static_cast<float>(window.height), 
            0.1f, 
            1000.0f
        );

        glEnable(GL_DEPTH_TEST);
	    glEnable(GL_CULL_FACE);

        debugAxes.render(projection * view);
        mybox.render(projection * view);


        window.swapBuffers();
        window.pollEvents();
    } 
    while (!window.shouldClose());

    std::cout << "Exiting.\n";
    return 0;
}



