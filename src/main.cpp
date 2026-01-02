#include <glm/detail/type_vec.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Window.hpp"
#include "ResourceManager.hpp"
#include "Camera.hpp"
#include "Cube.hpp"
#include "Perlin.hpp"
#include "Terrain.hpp"
#include "Shader.hpp"
#include <omp.h>

#include "GUIManager.hpp"
#include "Timer.hpp"
#include "WindowManager.hpp"
#include "InputManager.hpp"

// Should contain world objects (terrain, boxes, axes)
class Scene {
public:
    Scene() : terrain(3000.0f * glm::vec3(1,0,1), 300) {}

    void initialize() {
        terrainShader = std::make_shared<Shader>("../shaders/terrain.vert", "../shaders/terrain.frag");
        terrain.initialize(terrainShader, glm::vec3(0,0,0));
        debugAxes.initialize();
        mybox.initialize(glm::vec3(-200, 100, 0), glm::vec3(30,30,30));
    }

    void update(float dt) {
        terrain.update(dt);
    }

    void terrUpdateOffset(const glm::vec3& pos) { terrain.updateOffset(pos); }
    float terrGroundConstraint(glm::vec3& pos) { return terrain.groundHeightConstraint(pos); }
    void terrSetNoiseParams(int o, float p, int l) { terrain.setNoiseParams(o,p,l); }
    void terrSetPeakHeight(float h) { terrain.setPeakHeight(h); }

    void render(const glm::mat4& vp) {
        debugAxes.render(vp);
        mybox.render(vp);
        terrain.render(vp);
    }

private:
    std::shared_ptr<Shader> terrainShader;
    Terrain terrain;
    AxisXYZ debugAxes;
    Box mybox;
};



// we will just put the render code here organisation
class Renderer {
public:
    void renderScene(Scene& scene, Camera& camera, const WindowManager& window, float viewDist) {
        glClearColor(0.7f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(window.width()) / static_cast<float>(window.height()),
            10.0f, viewDist
        );

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        scene.render(projection * view);
    }
};

class Application {
public:
    Application() : windowManager(1366,768,"Wonderland"), camera(glm::vec3(0.0f,300.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,100.0f,0.0f) - glm::vec3(300.0f,300.0f,300.0f)), inputManager(windowManager, camera) {}

    bool initialize() {
        if (!windowManager.initialize())
            return false;
        gui.initialize(windowManager.getGLFWwindow());
        scene.initialize();
        return true;
    }

    int run() {
        float viewDist = 100000.0f;
        // UI state
        int octaves = 5;
        float persistence = 0.239f;
        int lacunarity = 2;
        float peakHeight = 800.0f;

        while (!windowManager.shouldClose()) {
            timer.tick();
            float dt = timer.getDeltaTime();

            windowManager.pollEvents();

            if (ImGui::IsKeyReleased(ImGuiKey_Tab)){
                windowManager.toggleCursorLock();
                std::cout << "menu toggled\n";
            }

            inputManager.update(dt);

            scene.terrUpdateOffset(camera.position);
            scene.update(dt);

            camera.setOnGround(scene.terrGroundConstraint(camera.position));

            // Render scene
            renderer.renderScene(scene, camera, windowManager, viewDist);

            // UI
            gui.newFrame();
            if (!windowManager.cursorLocked()) {
                ImGui::Begin("Terrain Parameters");
                ImGui::SetWindowSize(ImVec2(300, 150));
                ImGui::SliderInt("Octaves", &octaves, 1, 20);
                ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f);
                ImGui::SliderInt("Lacunarity", &lacunarity, 1, 10);
                ImGui::SliderFloat("Peak Height", &peakHeight, 0.0f, 2000.0f);
                ImGui::End();

                ImGui::Begin("View Parameters");
                ImGui::SetWindowSize(ImVec2(300, 150));
                ImGui::SliderFloat("View Distance", &viewDist, 500.0f, 10000.0f);
                ImGui::End();

                scene.terrSetNoiseParams(octaves, persistence, lacunarity);
                scene.terrSetPeakHeight(peakHeight);
            }
            gui.render();

            windowManager.swapBuffers();
        }

        gui.shutdown();
        std::cout << "Exiting.\n";
        return 0;
    }

private:
    WindowManager windowManager;
    Timer timer;
    Camera camera;
    Scene scene;
    InputManager inputManager;
    GUIManager gui;
    Renderer renderer;
};

int main() {
    Application app;
    if (!app.initialize()) return -1;
    return app.run();
}
