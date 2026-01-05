#include <glm/detail/type_vec.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "ArchTree.hpp"
#include "CheeseMoon.hpp"
#include "MushroomLight.hpp"
#include "MushroomLightSpawner.hpp"
#include "Phoenix.hpp"
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
#include "SkyBox.hpp"
#include <omp.h>

#include "GUIManager.hpp"
#include "Timer.hpp"
#include "InputManager.hpp"
#include "LightingParams.hpp"
#include "ShadowMap.hpp"
#include "PostProcessing.hpp"
// Should contain world objects (terrain, boxes, axes)
class Scene {
public:
    Scene() : 
    terrain(3000.0f * glm::vec3(1,0,1), 300)
    {
    }

    void initialize(const LightingParams& lightingParams) {
        // Initialize shadow mapping
        shadowMap.initialize();

        // Load shared resources for all model types ONCE before creating instances
        CheeseMoon::loadSharedResources();
        ArchTree::loadSharedResources();
        Phoenix::loadSharedResources();
        // Note: MushroomLight::loadSharedResources() is called in mushroomSpawner.initialize()

        terrainShader = std::make_shared<Shader>("../shaders/terrain.vert", "../shaders/terrain.frag");
        terrain.initialize(terrainShader, glm::vec3(0,0,0));
        debugAxes.initialize();
        mybox.initialize(glm::vec3(-200, 100, 0), glm::vec3(30,30,30));
        // Light source indicator - bright yellow box
        cheeseMoon.initialize(false);
        cheeseMoon.setPosition(lightingParams.lightPosition);
        cheeseMoon.setScale(150.0f * glm::vec3(1.0));
        cheeseMoon.setAlwaysLit(true);
        archTree.initialize(true);
        archTree.setPosition(glm::vec3(1000, 300, 0));
        phoenix.initialize(true);
        phoenix.setPosition(glm::vec3(500, 1500, 500));
        
        // Initialize mushroom spawner instead of a single mushroom
        // Spawns mushrooms in low terrain areas (height < -150)
        mushroomSpawner.initialize(&terrain, -150.0f, 150.0f, 2500.0f, 3000.0f, 0.3f);

        skybox.initialize(glm::vec3(0,0,0), 40000.0f * glm::vec3(1,1,1));
    }

    void update(float dt, Camera& camera) {
        terrain.update(dt);
        archTree.update(dt);
        phoenix.update(dt);
        mushroomSpawner.update(camera.getPosition(), dt);
        cheeseMoon.update(dt, camera.getPosition());
        skybox.update(camera.getPosition());
    }

    void terrUpdateOffset(const glm::vec3& pos) { terrain.updateOffset(pos); }
    float terrGroundConstraint(glm::vec3& pos) { return terrain.groundHeightConstraint(pos); }
    void terrSetNoiseParams(int o, float p, float l) { terrain.setNoiseParams(o,p,l); }
    void terrSetPeakHeight(float h) { terrain.setPeakHeight(h); }
    void terrSetWireframeMode(bool enabled) { terrain.setWireframeMode(enabled); }
    void updateLightIndicator(const glm::vec3& lightPos) { cheeseMoon.position = lightPos; }

    void render(const glm::mat4& vp, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane) {
        debugAxes.render(vp);
        mybox.render(vp);
        cheeseMoon.render(vp, lightingParams, cameraPos, farPlane);  // Visualize light source position
        
        // Bind shadow cubemap to texture unit 15 (high unit to avoid conflicts with material textures)
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap.depthCubemap);
        
        terrain.render(vp, lightingParams, cameraPos, farPlane);
        archTree.render(vp, lightingParams, cameraPos, farPlane);
        phoenix.render(vp, lightingParams, cameraPos, farPlane);
        mushroomSpawner.render(vp, lightingParams, cameraPos, farPlane);

        skybox.render(vp);
    }

    void renderDepthPass(const LightingParams& lightingParams) {
        // Render all shadow-casting objects to shadow map
        terrain.renderDepth(shadowMap.depthShader, lightingParams);
        archTree.renderDepth(shadowMap.depthShader);
        phoenix.renderDepth(shadowMap.depthShader);
        mushroomSpawner.renderDepth(shadowMap.depthShader);
    }

private:
    std::shared_ptr<Shader> terrainShader;
    Terrain terrain;
    AxisXYZ debugAxes;
    Box mybox;
    CheeseMoon cheeseMoon;
    ArchTree archTree;
    Phoenix phoenix;
    MushroomLightSpawner mushroomSpawner;
    SkyBox skybox;

public:
    ShadowMap shadowMap;
};



// we will just put the render code here organisation
class Renderer {
public:
    void renderScene(Scene& scene, Camera& camera, const Window& window, float viewDist, const LightingParams& lightingParams,
                     PostProcessing& postProcess, bool toonEnabled, bool lensFlareEnabled, float time) {
        // First pass: render depth map from light's perspective
        scene.shadowMap.beginRender();
        scene.shadowMap.setLightSpaceMatrices(lightingParams.lightPosition, 1.0f, viewDist, camera.getPosition(), lightingParams.fadeViewDistance, lightingParams.fadeDistance);
        scene.renderDepthPass(lightingParams);
        scene.shadowMap.endRender();

        // update post-processing framebuffer size
        postProcess.resize(window.width, window.height);

        postProcess.beginCapture();
        
        glViewport(0, 0, (int)window.width, (int)window.height);
        glClearColor(0.7f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.setZFar(viewDist);
        glm::mat4 view = camera.getViewMatrix();
        camera.setAspect(window.getAspectRatio());
        glm::mat4 projection = camera.getProjectionMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        // Enable double side rendering to avoid z-fighting
        

        scene.render(projection * view, lightingParams, camera.getPosition(), viewDist);

        postProcess.endCaptureAndRender(
            toonEnabled,
            lensFlareEnabled,
            lightingParams.lightPosition,
            view,
            projection,
            camera.getPosition(),
            time
        );
    }
};

class Application {
public:
    Application() : 
        mainWindow(1680,1050,"Wonderland"), 
        camera(glm::vec3(0.0f,300.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,100.0f,0.0f) - glm::vec3(300.0f,300.0f,300.0f)), 
        inputManager(mainWindow, camera) {}

    bool initialize() {
        mainWindow.initialize();
        gui.initialize(mainWindow.window);
        scene.initialize(lightingParams);
        postProcess.initialize(mainWindow.width, mainWindow.height);
        camera.setFov(45);
        camera.setZNear(10.0f);
        camera.setZFar(10000.0f);
        return true;
    }

    int run() {
        float viewDist = 100000.0f;
        float totalTime = 0.0f;
        // UI state
        int octaves = 5;
        float persistence = 0.503f;
        float lacunarity = 2;
        float peakHeight = 1100.0f;
        bool terrainWireframe = false;
        // Post-processing state
        bool toonShadingEnabled = false;
        bool lensFlareEnabled = true;

        while (!mainWindow.shouldClose()) {
            timer.tick();
            float dt = timer.getDeltaTime();

            mainWindow.pollEvents();

            if (ImGui::IsKeyReleased(ImGuiKey_Tab)) {
                mainWindow.toggleCursorLock();
                std::cout << "menu toggled\n";
            }

            inputManager.update(dt);

            scene.terrUpdateOffset(camera.position);
            camera.setOnGround(scene.terrGroundConstraint(camera.position));
            // only update scene after constrains enforced. Otherwise skybox jitter
            scene.update(dt, camera);

            totalTime += dt;

            // Render scene
            renderer.renderScene(scene, camera, mainWindow, viewDist, lightingParams, 
                                postProcess, toonShadingEnabled, lensFlareEnabled, totalTime);

            // UI
            gui.newFrame();
            if (!mainWindow.cursorLocked()) {
                ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
                ImGui::Begin("Terrain Parameters");
                ImGui::SliderInt("Octaves", &octaves, 1, 20);
                ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f);
                ImGui::SliderFloat("Lacunarity", &lacunarity, 1, 10);
                ImGui::SliderFloat("Peak Height", &peakHeight, 0.0f, 2000.0f);
                ImGui::Checkbox("Wireframe Mode", &terrainWireframe);
                ImGui::End();

                ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_FirstUseEver);
                ImGui::Begin("View Parameters");
                ImGui::SliderFloat("View Distance", &viewDist, 500.0f, 100000.0f);
                ImGui::End();

                ImGui::SetNextWindowSize(ImVec2(320, 340), ImGuiCond_FirstUseEver);
                ImGui::Begin("Lighting");
                
                // Light Position
                ImGui::Text("Light Position");
                ImGui::InputFloat3("Position", &lightingParams.lightPosition[0]);
                if (ImGui::Button("Light X -")) lightingParams.lightPosition.x -= 100.0f;
                ImGui::SameLine();
                if (ImGui::Button("Light X +")) lightingParams.lightPosition.x += 100.0f;
                if (ImGui::Button("Light Y -")) lightingParams.lightPosition.y -= 100.0f;
                ImGui::SameLine(); 
                if (ImGui::Button("Light Y +")) lightingParams.lightPosition.y += 100.0f;
                if (ImGui::Button("Light Z -")) lightingParams.lightPosition.z -= 100.0f;
                ImGui::SameLine();  
                if (ImGui::Button("Light Z +")) lightingParams.lightPosition.z += 100.0f;
                if (ImGui::Button("Set Light to Camera Position")) lightingParams.lightPosition = camera.getPosition();
                
                ImGui::Separator();
                
                // Light Color
                ImGui::Text("Light Color");
                ImGui::ColorEdit3("Color", &lightingParams.lightColor[0]);
                
                // Light Intensity (logarithmic scale for big values)
                ImGui::Text("Light Intensity");
                static float intensityLog = log10(lightingParams.lightIntensity.x);
                if (ImGui::SliderFloat("Intensity (10^x)", &intensityLog, 4.0f, 10.0f)) {
                    float intensity = pow(10.0f, intensityLog);
                    lightingParams.lightIntensity = glm::vec3(intensity);
                }
                
                ImGui::Separator();
                
                // Fade Parameters
                ImGui::Text("Object Fade");
                ImGui::SliderFloat("Fade Start Dist", &lightingParams.fadeViewDistance, 500.0f, 10000.0f);
                ImGui::SliderFloat("Fade Range", &lightingParams.fadeDistance, 100.0f, 2000.0f);
                
                ImGui::End();

                ImGui::SetNextWindowSize(ImVec2(300, 280), ImGuiCond_FirstUseEver);
                ImGui::Begin("Post-Processing Effects");
                
                ImGui::Checkbox("Toon Shading", &toonShadingEnabled);
                if (toonShadingEnabled) {
                    ImGui::SliderFloat("Edge Threshold", &postProcess.edgeThreshold, 0.01f, 1.0f);
                    ImGui::SliderInt("Color Levels", &postProcess.colorLevels, 2, 100);
                }
                
                ImGui::Separator();
                
                ImGui::Checkbox("Lens Flare", &lensFlareEnabled);
                if (lensFlareEnabled) {
                    ImGui::SliderFloat("Flare Intensity", &postProcess.lensFlareIntensity, 0.0f, 2.0f);
                    ImGui::SliderFloat("Flare Scale", &postProcess.lensFlareScale, 0.1f, 3.0f);
                    ImGui::SliderFloat("Streak Length", &postProcess.streakLength, 0.01f, 0.3f);
                    ImGui::SliderFloat("Halo Radius", &postProcess.haloRadius, 0.05f, 0.5f);
                    ImGui::SliderInt("Ghost Count", &postProcess.numGhosts, 0, 10);
                }
                
                ImGui::End();

                scene.terrSetNoiseParams(octaves, persistence, lacunarity);
                scene.terrSetPeakHeight(peakHeight);
                scene.terrSetWireframeMode(terrainWireframe);
                scene.updateLightIndicator(lightingParams.lightPosition);
            }
            gui.render();

            mainWindow.swapBuffers();
        }

        gui.shutdown();
        std::cout << "Exiting.\n";
        return 0;
    }

private:
    Window mainWindow;
    Timer timer;
    Camera camera;
    Scene scene;
    InputManager inputManager;
    GUIManager gui;
    Renderer renderer;
    LightingParams lightingParams;
    PostProcessing postProcess;
};

int main() {
    Application app;
    if (!app.initialize()) return -1;
    return app.run();
}
