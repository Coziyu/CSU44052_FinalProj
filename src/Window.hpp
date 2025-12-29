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

        // Container to calculate running average FPS
        float totalTime;
        float frameTimes[100];
        int frameTimeIndex; // Points to the latest updated frame time

        Window(int w, int h, const char* t);
        ~Window();
        void initialize();
        void swapBuffers();
        void pollEvents();
        bool shouldClose();
        bool isKeyPressed(int key);
        void recomputeFPS(float deltaTime);

};

#endif // WINDOW_HPP
