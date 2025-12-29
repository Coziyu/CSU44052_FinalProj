#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
    public:
        GLFWwindow* window;
        int width;
        int height;
        std::string title;

        Window(int w, int h, const char* t);
        ~Window();
        void swapBuffers();
        void pollEvents();
        bool shouldClose();
        bool isKeyPressed(int key);
};

#endif // WINDOW_HPP
