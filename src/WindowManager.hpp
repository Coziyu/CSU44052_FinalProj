#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#include "Window.hpp"
#include "glfw/glfw3.h"
#include <functional>

// -- For window io and GL context managemnet
class WindowManager {
public:
    WindowManager(int w = 1366, int h = 768, const char* title = "Wonderland") : window(w,h,title) {}

    bool initialize() {
        window.initialize();
        return true;
    }

    void pollEvents() { window.pollEvents(); }
    void swapBuffers() { window.swapBuffers(); }
    bool shouldClose() const { return window.shouldClose(); }
    void toggleCursorLock() { window.toggleCursorLock(); }
    bool cursorLocked() const { return window.cursorLocked(); }
    void recomputeFPS(float dt) { window.recomputeFPS(dt); }
    void processInput() { window.processInput(); }
    void registerMouseCallback(std::function<void(double,double,bool)> cb) { window.registerMouseCallback(cb); }
    GLFWwindow* getGLFWwindow() { return window.window; }
    int width() const { return window.width; }
    int height() const { return window.height; }

private:
    Window window;
};

#endif // WINDOWMANAGER_HPP
