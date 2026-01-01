#include <glm/detail/type_vec.hpp>
#include <iostream>
#include <glm/gtc//matrix_transform.hpp>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "core/Window.hpp"
#include "core/ResourceManager.hpp"
#include "Camera.hpp"
#include "Cube.hpp"
#include "Perlin.hpp"
#include "Terrain.hpp"
#include "Shader.hpp"
#include <omp.h>
// TODO: MAKE AN OPTION TO ENABLE OR DISABLE OPENMP, IF ENABLED, SET TERRAIN RESOLUTION TO 300, ELSE 100
int main() {
    Window window(1366, 768, "Wonderland");
    window.initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui_ImplGlfw_InitForOpenGL(window.window, true);
    ImGui_ImplOpenGL3_Init();

    Camera camera(
        glm::vec3(0.0f, 300.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 100.0f, 0.0f) - glm::vec3(300.0f, 300.0f, 300.0f)
    );
    window.registerMouseCallback(
        [&camera](double x, double y, bool cursorLocked) { camera.handleMouseInput(x, y, cursorLocked); }
    );
    
    ResourceManager& resourceManager = ResourceManager::getInstance();

    std::shared_ptr<Shader> terrainShader = std::make_shared<Shader>(
        "../shaders/terrain.vert",
        "../shaders/terrain.frag"
    );
    int resolution = 300;
    Terrain terrain = Terrain(3000.0f * glm::vec3(1, 0, 1), resolution); 
    terrain.initialize(terrainShader, glm::vec3(0, 0, 0));


    // A coordinate system
    AxisXYZ debugAxes;
    debugAxes.initialize();

    // A default box
    Box mybox;
    mybox.initialize(
        glm::vec3(-200, 100, 0), // translation
        glm::vec3(30, 30, 30) // scale
    );

    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    float viewDist = 100000.0f;
    do {
        float currTime = (glfwGetTime());
        deltaTime = currTime - lastTime;
        lastTime = currTime;

        // -- Input process and update
        window.pollEvents();
        
        if (ImGui::IsKeyReleased(ImGuiKey_Tab)){
            window.toggleCursorLock();
            std::cout << "menu toggled\n";
        }
        
        if (window.cursorLocked()){
            camera.processInput(window, deltaTime);
            camera.update(deltaTime);
            window.recomputeFPS(deltaTime);
            window.processInput();

        }
        camera.setOnGround(terrain.groundHeightConstraint(camera.position));

        terrain.updateOffset(camera.position);
        terrain.update(deltaTime);

        

        // Actual render of scene
        glClearColor(0.7f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(window.width) / static_cast<float>(window.height),
            10.0f, viewDist
        );

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        debugAxes.render(projection * view);
        mybox.render(projection * view);
        terrain.render(projection * view);

        // -- Render IMGUI next````````````````''''
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if (!window.cursorLocked()) {
            static int octaves = 5;
            static float persistence = 0.239f;
            static int lacularity = 2.0f;
            static float peakHeight = 800.0f;
            ImGui::Begin("Terrain Parameters");
            // Size of window
            ImGui::SetWindowSize(ImVec2(300, 150));
            ImGui::SliderInt("Octaves", &octaves, 1, 20);
            ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f);
            ImGui::SliderInt("Lacunarity", &lacularity, 1, 10);
            ImGui::SliderFloat("Peak Height", &peakHeight, 0.0f, 2000.0f);

            ImGui::End();

            ImGui::Begin("View Parameters");
            ImGui::SetWindowSize(ImVec2(300, 150));
            ImGui::SliderFloat("View Distance", &viewDist, 500.0f, 10000.0f);
            ImGui::End();

            terrain.setNoiseParams(octaves, persistence, lacularity);
            terrain.setPeakHeight(peakHeight);
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        
        window.swapBuffers();

    } while (!window.shouldClose());

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    std::cout << "Exiting.\n";

//   // Test rendering a perlin texture to an image.
//   testRenderPerlinNoise();

    return 0;
}
