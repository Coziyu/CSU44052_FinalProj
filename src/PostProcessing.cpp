#include "PostProcessing.hpp"
#include <iostream>

PostProcessing::PostProcessing() {}

PostProcessing::~PostProcessing() {
    deleteFramebuffers();
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}

void PostProcessing::initialize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    
    createFramebuffers(width, height);
    createFullscreenQuad();
    
    // Load shaders
    toonShader = std::make_shared<Shader>("../shaders/postprocess.vert", "../shaders/toon_postprocess.frag");
    lensFlareShader = std::make_shared<Shader>("../shaders/postprocess.vert", "../shaders/lens_flare.frag");
    passthroughShader = std::make_shared<Shader>("../shaders/postprocess.vert", "../shaders/passthrough.frag");
}

void PostProcessing::resize(int width, int height) {
    if (width == screenWidth && height == screenHeight) return;
    
    screenWidth = width;
    screenHeight = height;
    deleteFramebuffers();
    createFramebuffers(width, height);
}

void PostProcessing::createFramebuffers(int width, int height) {
    // Create main scene framebuffer
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    
    // Color texture
    glGenTextures(1, &sceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTexture, 0);
    
    // Depth texture (for edge detection)
    glGenTextures(1, &sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Scene framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::createFullscreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

void PostProcessing::deleteFramebuffers() {
    if (sceneFBO) {
        glDeleteFramebuffers(1, &sceneFBO);
        glDeleteTextures(1, &sceneColorTexture);
        glDeleteTextures(1, &sceneDepthTexture);
        sceneFBO = 0;
        sceneColorTexture = 0;
        sceneDepthTexture = 0;
    }
}

void PostProcessing::beginCapture() {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessing::endCaptureAndRender(
    bool toonShadingEnabled,
    bool lensFlareEnabled,
    const glm::vec3& lightPosition,
    const glm::mat4& viewMatrix,
    const glm::mat4& projectionMatrix,
    const glm::vec3& cameraPosition,
    float time
) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    
    // Calculate light screen position for lens flare
    glm::vec4 lightClip = projectionMatrix * viewMatrix * glm::vec4(lightPosition, 1.0f);
    glm::vec2 lightScreenPos(0.0f);
    bool lightVisible = false;
    float lightOcclusion = 0.0f;  // 0 = fully blocked, 1 = fully visible
    
    if (lightClip.w > 0.0f) {
        glm::vec3 lightNDC = glm::vec3(lightClip) / lightClip.w;
        if (lightNDC.x >= -1.0f && lightNDC.x <= 1.0f && 
            lightNDC.y >= -1.0f && lightNDC.y <= 1.0f &&
            lightNDC.z >= -1.0f && lightNDC.z <= 1.0f) {
            lightScreenPos = glm::vec2(lightNDC.x * 0.5f + 0.5f, lightNDC.y * 0.5f + 0.5f);
            // ChatGPT assisted with this part of the code to hide len flare when occluded
            // check depth occlusion by sampling depth buffer around light pos
            // sample multiple points for smooth transitions
            int sampleCount = 0;
            int visibleSamples = 0;
            const int sampleRadius = 2;
            
            for (int dx = -sampleRadius; dx <= sampleRadius; dx++) {
                for (int dy = -sampleRadius; dy <= sampleRadius; dy++) {
                    int px = static_cast<int>(lightScreenPos.x * screenWidth) + dx * 2;
                    int py = static_cast<int>(lightScreenPos.y * screenHeight) + dy * 2;
                    
                    if (px >= 0 && px < screenWidth && py >= 0 && py < screenHeight) {
                        float depthSample;
                        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
                        glReadPixels(px, py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthSample);
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        
                        // Convert light NDC z to depth buffer value (0 to 1)
                        float lightDepth = lightNDC.z * 0.5f + 0.5f;
                        
                        // If depth buffer value is greater (farther) than light, light is visible
                        // Add small bias to avoid z-fighting
                        if (depthSample > lightDepth - 0.001f || depthSample > 0.9999f) {
                            visibleSamples++;
                        }
                        sampleCount++;
                    }
                }
            }
            
            if (sampleCount > 0) {
                lightOcclusion = static_cast<float>(visibleSamples) / static_cast<float>(sampleCount);
            }
            
            lightVisible = lightOcclusion > 0.0f;
        }
    }
    
    // Apply toon shading if enabled
    if (toonShadingEnabled) {
        toonShader->use();
        toonShader->setUniInt("screenTexture", 0);
        toonShader->setUniInt("depthTexture", 1);
        toonShader->setUniFloat("edgeThreshold", edgeThreshold);
        toonShader->setUniInt("colorLevels", colorLevels);
        toonShader->setUniVec2("screenSize", glm::vec2(screenWidth, screenHeight));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
    } else {
        passthroughShader->use();
        passthroughShader->setUniInt("screenTexture", 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
    }
    
    // fullscreen quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // render lens flare on top of whatever
    if (lensFlareEnabled && lightVisible) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);  
        
        lensFlareShader->use();
        lensFlareShader->setUniVec2("lightScreenPos", lightScreenPos);
        lensFlareShader->setUniVec2("screenSize", glm::vec2(screenWidth, screenHeight));
        lensFlareShader->setUniFloat("intensity", lensFlareIntensity * lightOcclusion); 
        lensFlareShader->setUniFloat("scale", lensFlareScale);
        lensFlareShader->setUniFloat("time", time);
        lensFlareShader->setUniFloat("aspectRatio", (float)screenWidth / (float)screenHeight);
        lensFlareShader->setUniFloat("streakLength", streakLength);
        lensFlareShader->setUniFloat("haloRadius", haloRadius);
        lensFlareShader->setUniInt("numGhosts", numGhosts);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisable(GL_BLEND);
    }
    
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}
