#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "Core/API/Texture.h"

#include <string>
#include <vector>
using namespace std;

namespace Desperado {
    class dlldecl TRStransform {
    public:
        TRStransform() {};
        TRStransform(glm::vec3 translate) :translate(translate) {};
        TRStransform(glm::vec3 translate, glm::vec3 scale) :translate(translate), scale(scale) {};
        TRStransform(glm::vec3 translate, glm::vec3 scale, glm::vec3 rotateAxis, float rotateAngle) :translate(translate), scale(scale), rotateAxis(rotateAxis), rotateAngle(rotateAngle) {};
        glm::vec3 translate = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
        glm::vec3 rotateAxis = glm::vec3(0.0, 0.0, 0.0);
        float rotateAngle = 0.0;
    };

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    struct Material {
        glm::vec3 Kd;
        glm::vec3 Ks;
        glm::vec3 Ka;
        float Shininess;
    };

    class dlldecl Mesh {
    public:
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<std::shared_ptr<Texture>>      textures;
        Material             mat;
        unsigned int VAO;

        static unsigned int mesh_id_count;

        TRStransform transform;

        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<std::shared_ptr<Texture>> textures, TRStransform trans);
        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<std::shared_ptr<Texture>> textures, Material mat, TRStransform trans);

        void Draw(std::shared_ptr<Shader> pShader);

        unsigned int getVBO() {
            return VBO;
        }
        unsigned int getEBO() {
            return EBO;
        }
        unsigned int getMeshID() {
            return mesh_ID;
        }

        static shared_ptr<Mesh> createCube(TRStransform transform);

    private:
        unsigned int VBO, EBO;
        unsigned int mesh_ID;

        void setupMesh();
    };
}
#endif