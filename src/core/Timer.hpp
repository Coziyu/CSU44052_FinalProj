#ifndef TIMER_HPP
#define TIMER_HPP

#include <GLFW/glfw3.h>
class Timer {
public:
    void tick() {
        float now = static_cast<float>(glfwGetTime());
        delta = now - last;
        last = now;
    }
    float getDeltaTime() const { return delta; }
private:
    float last = 0.0f;
    float delta = 0.0f;
};

#endif // TIMER_HPP
