#pragma once
#include "scene.h"
#include "shader.h"

#include <iostream>
#include <map>
#include <optixu/optixpp.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>


namespace Desperado {
    class dlldecl OptixContext {
    public:
        using UniquePtr = std::unique_ptr<OptixContext>;
        struct MeshBuffers
        {
            optix::Buffer vertices;
            optix::Buffer indices;
            //optix::Buffer tri_indices;
            //optix::Buffer mat_indices;
            //optix::Buffer positions;
            //optix::Buffer normals;
            //optix::Buffer texcoords;
        };
        static UniquePtr create(const std::string& mSampleName, const std::string& mCuName, Scene::SharedPtr pScene, Camera::SharedPtr pCamera, uint32_t width, uint32_t height);
        ~OptixContext();

        optix::Buffer getBuffer(std::string bufName);
        void destroyContext();
        void registerExitHandler();
        void createContext();
        void setupLights();
        void loadGeometry();
        void setupCamera();
        void updateCamera();

        void validate();
        void launch();

        void setContextTextureSampler(const Texture::SharedPtr pTex, const string& name);
        void createTexIdBuffer(const std::vector<Texture::SharedPtr> textures);
    private:
        OptixContext(const std::string& mSampleName, const std::string& mCuName, Scene::SharedPtr pScene, Camera::SharedPtr pCamera, uint32_t width, uint32_t height);
        std::string mSampleName;
        std::string mCuName;
        std::string getCuFilename(std::string const& name);

        std::map<const Texture::SharedPtr, optix::TextureSampler> m_texture_sampler;
        optix::TextureSampler m_empty_sampler = 0;

        optix::Program _mesh_intersect = 0;
        optix::Program _mesh_bounds = 0;
        optix::Program _material_closest_hit = 0;
        optix::Program _material_any_hit = 0;
        optix::Program _material_shadow_any_hit = 0;

        optix::GeometryInstance createParallelogram(const optix::float3& anchor,
            const optix::float3& offset1, const optix::float3& offset2);
        optix::Geometry getMeshGeometry(const shared_ptr<Model> model);
        optix::Geometry getMeshGeometry(const shared_ptr<Mesh> mesh);

        optix::TextureSampler getTextureSampler(const Texture::SharedPtr pTex);
        optix::TextureSampler getEmptySampler();       
        optix::TextureSampler addTextureSampler(const Texture::SharedPtr pTex);

    private:
        void initProgram();
        void initSceneGeometry();
        void initSceneLights();
        void initSceneCamera();

    public:
        inline uint32_t getWidth() const { return m_width; }
        inline uint32_t getHeight() const { return m_height; }
        inline optix::Context getContext() const { return m_context; }

    private:
        optix::Context        m_context = 0;

        uint32_t       m_width;
        uint32_t       m_height;
        bool           m_use_pbo = true;

        int            m_frame_number = 1;
        int            m_sqrt_num_samples = 1;
        int            m_rr_begin_depth = 1;

        optix::Program        m_pgram_intersection = 0;
        optix::Program        m_pgram_bounding_box = 0;

        optix::Program        m_closest_hit_emitter = 0;

        optix::Program        m_triangle_intersection = 0;
        optix::Program        m_triangle_bounds = 0;
        optix::Program        m_triangle_attribute = 0;

        optix::Program        m_closest_hit = 0;
        optix::Program        m_any_hit = 0;
        optix::Program		  m_any_hit_shadow = 0;

        std::vector<optix::GeometryInstance> m_light_gis;
        std::vector<optix::GeometryInstance> m_gis;

        Scene::SharedPtr      mpScene;
        Camera::SharedPtr     mpCamera;
        // Camera state
        optix::float3         m_camera_up;
        optix::float3         m_camera_lookat;
        optix::float3         m_camera_eye;
        optix::Matrix4x4      m_camera_rotate;    
    };
}

