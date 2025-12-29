#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>
#include <vector>

const int FRAMETIME_SAMPLES = 20;

class Window {
    public:
        GLFWwindow* window;
        int width;
        int height;
        std::string title;

        // Container to calculate running average FPS
        float totalTime;
        float frameTimes[FRAMETIME_SAMPLES];
        int frameTimeIndex; // Points to the latest updated frame time

        // Generic callback handlers container
        std::vector<std::function<void(double, double)>> mouseMoveCallbacks;

        Window(int w, int h, const char* t);
        ~Window();
        void initialize();
        
        void swapBuffers();
        void pollEvents();
        void processInput();

        bool shouldClose();
        bool isKeyPressed(int key);
        void recomputeFPS(float deltaTime);

        void registerMouseCallback(const std::function<void(double, double)>& callback);

    private:
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos);
        
        void onKeyDown(int key, int scancode, int action, int mods);
        void onMouseMove(double xpos, double ypos);

};

#endif // WINDOW_HPP
