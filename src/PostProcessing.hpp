#ifndef POSTPROCESSING_HPP
#define POSTPROCESSING_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include "core/Shader.hpp"
// [ACKN] ChatGPT assisted with creating the class to capture framebuffer for post-processing
/**
 * @brief Handles post-processing effects including toon shading and lens flare
 */
class PostProcessing {
public:
    PostProcessing();
    ~PostProcessing();

    void initialize(int width, int height);
    void resize(int width, int height);
    
    // Begin rendering to the offscreen framebuffer
    void beginCapture();
    
    // End capturing and render the final result with effects applied
    void endCaptureAndRender(
        bool toonShadingEnabled,
        bool lensFlareEnabled,
        const glm::vec3& lightPosition,
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        const glm::vec3& cameraPosition,
        float time
    );

    // Settings
    float edgeThreshold = 0.841f;
    int colorLevels = 32;
    float lensFlareIntensity = 0.5f;
    float lensFlareScale = 1.0f;
    float streakLength = 0.08f;
    float haloRadius = 0.15f;
    int numGhosts = 5;

private:
    void createFramebuffers(int width, int height);
    void createFullscreenQuad();
    void deleteFramebuffers();

    // Main scene framebuffer
    GLuint sceneFBO = 0;
    GLuint sceneColorTexture = 0;
    GLuint sceneDepthTexture = 0;

    // Fullscreen quad
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // Shaders
    std::shared_ptr<Shader> toonShader;
    std::shared_ptr<Shader> lensFlareShader;
    std::shared_ptr<Shader> passthroughShader;

    int screenWidth = 0;
    int screenHeight = 0;
};

#endif // POSTPROCESSING_HPP
