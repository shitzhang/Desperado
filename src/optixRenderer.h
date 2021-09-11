#pragma once
#include "scene.h"
#include "shader.h"

#include <iostream>
#include <map>
#include <optixu/optixpp.h>
#include <optixu/optixu_math_namespace.h>



class OptixRenderer {
    public:
        OptixRenderer(std::string const& ptxDir);
        ~OptixRenderer();
        void init(uint32_t w, uint32_t h);
        void exit();

    private:
        std::string mPtxDir;
        std::string getPtxFilename(std::string const& name);

        std::map<const Texture*, optix::TextureSampler> _texture_sampler;
        optix::TextureSampler _empty_sampler = 0;

        optix::Program _mesh_intersect = 0;
        optix::Program _mesh_bounds = 0;
        optix::Program _material_closest_hit = 0;
        optix::Program _material_any_hit = 0;
        optix::Program _material_shadow_any_hit = 0;

        optix::Geometry getMeshGeometry(const shared_ptr<Model> model );
        optix::Geometry getMeshGeometry(const shared_ptr<Mesh> mesh);

        optix::TextureSampler getTextureSampler(const Texture* tex);
        optix::TextureSampler getEmptySampler();


    private:
        enum BackgroundMode { BLACK, PROCEDURAL_SKY, CUBEMAP, HDRMAP } backgroundMode = PROCEDURAL_SKY;

        void initSceneGeometry(const Scene& scene);
        void initSceneLights(const Scene& scene);
        void initSceneCamera();

    public:
        inline uint32_t getWidth() const { return width; }
        inline uint32_t getHeight() const { return height; }

    private:
        uint32_t width, height;

        optix::Context context = 0;

        GLuint screenVbo = 0;

        uint32_t nSamplesSqrt = 1;

        struct Cubemap {
            uint32_t width;
            std::vector<unsigned char> front;
            std::vector<unsigned char> back;
            std::vector<unsigned char> left;
            std::vector<unsigned char> right;
            std::vector<unsigned char> top;
            std::vector<unsigned char> bottom;
        } cubemap;

        struct Hdrmap {
            uint32_t height;
            std::vector<float> texture;
        } hdrmap;

    public:
        void renderScene(const Scene& scene, const Camera& camera);
        void display();
        std::vector<float> getResult();
        // void renderCurrentToFile(std::string filename);
        void setCubemap(std::string const& front, std::string const& back, std::string const& top,
            std::string const& bottom, std::string const& left, std::string const& right);

        void setHdrmap(std::string const& map);
        inline void setBlackBackground() { backgroundMode = BLACK; };
        inline void setProceduralSkyBackground() { backgroundMode = PROCEDURAL_SKY; };
 };

