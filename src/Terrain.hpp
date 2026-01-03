#ifndef TERRAIN_HPP
#define TERRAIN_HPP
#include "Perlin.hpp"
#include "Shader.hpp"
#include "Entities.hpp"
#include "utils.hpp"
#include "LightingParams.hpp"
#include <glm/detail/type_vec.hpp>
#include <memory>

class Terrain : public DynamicEntity {
    protected:
        std::vector<glm::vec3> vertex_buffer_data;
        std::vector<glm::vec3> normal_buffer_data;
        std::vector<unsigned int> index_buffer_data;

        std::vector<glm::vec3> face_normals;
        
        // OpenGL Buffers
        GLuint vertexArrayID;
        GLuint vertexBufferID;
        GLuint indexBufferID;
        GLuint normalBufferID;

        // 
        PerlinNoise pn;
        glm::vec3 offset; // This parameter is used to "move" the terrain around by shifting the noise map
        // Parameters of the terrain
        int resolution;
        int octaves;
        float persistence;
        float lacunarity;
        float peakHeight;

        bool modeWireframe;

        // For computing the special scale factor
        float fov;
        glm::vec3 specialScale; // To ensure when flying up, the terrain scales correctly to fill fov
        float speedScaleFactor;
        float consistencyFactor;


    public:
        Terrain(glm::vec3 _scale, int _resolution);
        void render(glm::mat4 vp, const LightingParams& lightingParams, glm::vec3 cameraPos, float farPlane) override;
        void renderDepth(std::shared_ptr<Shader> depthShader, const LightingParams& lightingParams);
        void update(float deltaTime) override;
        void updateOffset(glm::vec3 newOffset);
        void initialize(std::shared_ptr<Shader> shaderptr, glm::vec3 position);
        void setNoiseParams(int octaves, float persistence, float lacunarity);
        void setPeakHeight(float peakHeight);
        float getCenterHeight();

        bool groundHeightConstraint(glm::vec3 &position);
        void setWireframeMode(bool enabled);

        // Query terrain height/normal at arbitrary world coordinates
        float getHeightAt(float worldX, float worldZ) const;
        glm::vec3 getNormalAt(float worldX, float worldZ) const;

        // Expose terrain parameters for external queries
        const PerlinNoise& getPerlinNoise() const { return pn; }
        int getOctaves() const { return octaves; }
        float getPersistence() const { return persistence; }
        float getLacunarity() const { return lacunarity; }
        float getPeakHeight() const { return peakHeight; }
        glm::vec3 getOffset() const { return offset; }
        float getConsistencyFactor() const { return consistencyFactor; }
};

#endif // TERRAIN_HPP
