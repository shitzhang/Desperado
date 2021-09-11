#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>
using namespace std;

class TRStransform {
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
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    //glm::vec3 Tangent;
    // bitangent
    //glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct Material {
    glm::vec3 Kd;
    glm::vec3 Ks;
    glm::vec3 Ka;
    float Shininess;
};


class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    Material             mat;
    unsigned int VAO;

    TRStransform transform;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,TRStransform trans):transform(trans)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, Material mat,TRStransform trans) :transform(trans)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->mat = mat;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader& shader)
    {
        //shader.setVec3("Kd", this->mat.Kd);
        //shader.setVec3("Ks", this->mat.Ks);
        //shader.setVec3("Ka", this->mat.Ka);
        //shader.setFloat("Shininess", this->mat.Shininess);

        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); 
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); 
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); 
            else if (name == "texture_height")
                number = std::to_string(heightNr++); 


            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            //shader.setInt(name + number, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

    unsigned int getVBO() {
        return VBO;
    }
    unsigned int getEBO() {
        return EBO;
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        //glEnableVertexAttribArray(3);
        //glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        //glEnableVertexAttribArray(4);
        //glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};


static shared_ptr<Mesh> cube(TRStransform transform) {
    const glm::vec3 positions[24] = {
            // Front face
            glm::vec3(-1.0, -1.0, 1.0),
            glm::vec3(1.0, -1.0, 1.0),
            glm::vec3(1.0, 1.0, 1.0),
            glm::vec3(-1.0, 1.0, 1.0),

            // Back face
            glm::vec3(-1.0, -1.0, -1.0),
            glm::vec3(-1.0, 1.0, -1.0),
            glm::vec3(1.0, 1.0, -1.0),
            glm::vec3(1.0, -1.0, -1.0),

            // Top face
            glm::vec3(-1.0, 1.0, -1.0),
            glm::vec3(-1.0, 1.0, 1.0),
            glm::vec3(1.0, 1.0, 1.0),
            glm::vec3(1.0, 1.0, -1.0),

            // Bottom face
            glm::vec3(-1.0, -1.0, -1.0),
            glm::vec3(1.0, -1.0, -1.0),
            glm::vec3(1.0, -1.0, 1.0),
            glm::vec3(-1.0, -1.0, 1.0),

            // Right face
            glm::vec3(1.0, -1.0, -1.0),
            glm::vec3(1.0, 1.0, -1.0),
            glm::vec3(1.0, 1.0, 1.0),
            glm::vec3(1.0, -1.0, 1.0),

            // Left face
            glm::vec3(-1.0, -1.0, -1.0),
            glm::vec3(-1.0, -1.0, 1.0),
            glm::vec3(-1.0, 1.0, 1.0),
            glm::vec3(-1.0, 1.0, -1.0),
    };
    vector<unsigned int> indices = {
            0, 1, 2, 0, 2, 3,    // front
            4, 5, 6, 4, 6, 7,    // back
            8, 9, 10, 8, 10, 11,   // top
            12, 13, 14, 12, 14, 15,   // bottom
            16, 17, 18, 16, 18, 19,   // right
            20, 21, 22, 20, 22, 23,   // left
    };
    vector<Vertex> vertices;
    for (int i = 0; i < 24; i++) {
        Vertex v;
        v.Position = positions[i];
        vertices.push_back(v);
    }
    vector<Texture> textures;
    //return Mesh(vertices, indices, textures, transform);
    return make_shared<Mesh>(vertices, indices, textures, transform);
}

#endif