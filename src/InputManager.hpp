// handles mouse/keyboard bindings and camera interaction
#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include "Camera.hpp"
#include "WindowManager.hpp"

class InputManager {
public:
    InputManager(WindowManager& wm, Camera& cam) : window(wm), camera(cam) {
        window.registerMouseCallback([this](double x, double y, bool lock){ camera.handleMouseInput(x,y,lock); });
    }

    void update(float dt) {
        if (window.cursorLocked()) {
            camera.processInput(*reinterpret_cast<Window*>(&window), dt);
            camera.update(dt);
            window.recomputeFPS(dt);
            window.processInput();
        }
    }

private:
    WindowManager& window;
    Camera& camera;
};

#endif // INPUTMANAGER_HPP
