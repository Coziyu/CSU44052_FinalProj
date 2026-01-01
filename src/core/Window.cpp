#include "Window.hpp"
#include <sstream>
#include <GLFW/glfw3.h>

Window::Window(int w, int h, const char* t) : width(w), height(h), title(t) {};

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

    
    glfwFocusWindow(window); //Do this to set cursor pos. otherwise it fails silently
    glfwSetCursorPos(window, static_cast<int>(width / 2), static_cast<int>(height / 2));
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    isCursorLocked = true;

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetCursorPosCallback(window, glfwMouseCallback);
    // Dynamic viewport adjustment
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
        glViewport(0, 0, w, h); // TODO: Consider whether to update stored width and height
    });

    totalTime = 0.0f;
    frameTimeIndex = 0;
    for (int i = 0; i < FRAMETIME_SAMPLES; ++i) {
        frameTimes[i] = 0.0f;
    }
}

void Window::swapBuffers() {
    glfwSwapBuffers(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::processInput() {
    if (isKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(window);
}

bool Window::isKeyPressed(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::isKeyReleased(int key) {
    return glfwGetKey(window, key) == GLFW_RELEASE;
}

void Window::recomputeFPS(float deltaTime) {
    // Compute average FPS over the last 100 frames
    int newIndex = (frameTimeIndex + 1) % FRAMETIME_SAMPLES;
    totalTime -= frameTimes[newIndex];
    frameTimes[newIndex] = deltaTime;
    totalTime += frameTimes[newIndex];
    frameTimeIndex = newIndex;

    // Set window title to show FPS
    float averageFrameTime = totalTime / FRAMETIME_SAMPLES;
    if (averageFrameTime == 0.0f) return;
    float fps = 1.0f / averageFrameTime;

    std::stringstream ss;
    ss << title << " - FPS: " << static_cast<int>(fps);

    glfwSetWindowTitle(window, ss.str().c_str());

}

void Window::registerMouseCallback(const std::function<void(double, double, bool)>& callback) {
    mouseMoveCallbacks.push_back(callback);
}

void Window::toggleCursorLock() {
    isCursorLocked = !isCursorLocked;
    if (isCursorLocked) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool Window::cursorLocked() {
    return isCursorLocked;
}

void Window::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (!win) {
        std::cerr << "Window pointer is null in key callback.\n";
        return;
    }
    win->onKeyDown(key, scancode, action, mods);
}

void Window::glfwMouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (!win) {
        std::cerr << "Window pointer is null in mouse callback.\n";
        return;
    }
    win->onMouseMove(xpos, ypos);
}
void Window::onKeyDown(int key, int scancode, int action, int mods) {


    std::cout << "Key event: key=" << key 
              << ", scancode=" << scancode 
              << ", action=" << action 
              << ", mods=" << mods << std::endl;
}

void Window::onMouseMove(double xpos, double ypos) {
    for (const auto& callback : mouseMoveCallbacks) {
        callback(xpos, ypos, isCursorLocked);
    }
}
