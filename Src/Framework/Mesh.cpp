#include "stdafx.h"
#include "mesh.h"

namespace Desperado {

    unsigned int Mesh::mesh_id_count = 0;

    Mesh::Mesh(std::vector<Vertex> vertices, vector<unsigned int> indices, vector<std::shared_ptr<Texture>> textures, TRStransform trans) :transform(trans)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        mesh_ID = mesh_id_count++;

        setupMesh();
    }
    Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<std::shared_ptr<Texture>> textures, Material mat, TRStransform trans) :transform(trans)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->mat = mat;

        mesh_ID = mesh_id_count++;

        setupMesh();
    }

    void Mesh::Draw(std::shared_ptr<Shader> pShader)
    {
        pShader->setVec3("Kd", this->mat.Kd);
        pShader->setVec3("Ks", this->mat.Ks);
        pShader->setVec3("Ka", this->mat.Ka);
        pShader->setFloat("Shininess", this->mat.Shininess);

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            //glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i]->getDesc();
            if (name == "diffuse_map")
                number = std::to_string(diffuseNr++);
            else if (name == "specular_map")
                number = std::to_string(specularNr++);
            else if (name == "normal_map")
                number = std::to_string(normalNr++);
            else if (name == "height_map")
                number = std::to_string(heightNr++);

            //pShader->setInt(name + number, i);
            pShader->setTexture2D(name + number, textures[i]);
            //textures[i]->bind();
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

    void Mesh::setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

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
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
    shared_ptr<Mesh> Mesh::createCube(TRStransform transform) {
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
        vector<std::shared_ptr<Texture>> textures;
        return make_shared<Mesh>(vertices, indices, textures, transform);
    }
}