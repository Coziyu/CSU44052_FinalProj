#include "Window.hpp"
#include <sstream>

Window::Window(int w, int h, const char* t) : width(w), height(h), title(t) {
    initialize();
};

Window::~Window(){
    glfwDestroyWindow(window);
}

void Window::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    // Dynamic viewport adjustment
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
        glViewport(0, 0, w, h); // TODO: Consider whether to update stored width and height
    });

    totalTime = 0.0f;
    frameTimeIndex = 0;
    for (int i = 0; i < 100; ++i) {
        frameTimes[i] = 0.0f;
    }
}

void Window::swapBuffers() {
    glfwSwapBuffers(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(window);
}

bool Window::isKeyPressed(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::recomputeFPS(float deltaTime) {
    // Compute average FPS over the last 100 frames
    int oldestIndex = (frameTimeIndex + 1) % 100;
    totalTime -= frameTimes[oldestIndex];
    frameTimes[oldestIndex] = deltaTime;
    totalTime += frameTimes[oldestIndex];
    frameTimeIndex = oldestIndex;

    // Set window title to show FPS
    float averageFrameTime = totalTime / 100.0f;
    float fps = 1.0f / averageFrameTime;

    std::stringstream ss;
    ss << title << " - FPS: " << fps;

    glfwSetWindowTitle(window, ss.str().c_str());

}
