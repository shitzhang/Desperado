#include "stdafx.h"
#include "Model.h"

namespace Desperado
{
	// constructor, expects a filepath to a 3D model.
	Model::Model(string const& path, TRStransform transform, bool gamma) : gammaCorrection(gamma),
		transform(transform)
	{
		loadModel(path);
	}

	// draws the model, and thus all its meshes
	void Model::Draw(std::shared_ptr<Shader> pShader)
	{
		for (unsigned int i = 0; i < p_meshes.size(); i++)
		{
			p_meshes[i]->Draw(pShader);
		}
	}

	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void Model::loadModel(string const& path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void Model::processNode(aiNode* node, const aiScene* scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			p_meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	std::shared_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<std::shared_ptr<Texture>> textures;

		// walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			// texture coordinates
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		Material mat = loadMaterial(material);
		// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN

		// 1. diffuse maps
		vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTextures(
			material, aiTextureType_DIFFUSE, "diffuse_map");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTextures(
			material, aiTextureType_SPECULAR, "specular_map");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTextures(
			material, aiTextureType_NORMALS, "normal_map");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		vector<std::shared_ptr<Texture>> heightMaps =
			loadMaterialTextures(material, aiTextureType_HEIGHT, "height_map");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// return a mesh object created from the extracted mesh data
		//return Mesh(vertices, indices, textures, mat, transform);
		return std::make_shared<Mesh>(vertices, indices, textures, mat, transform);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<std::shared_ptr<Texture>> textures;
		if (type == aiTextureType_DIFFUSE && mat->GetTextureCount(type) == 0)
		{
			//Texture texture;

			aiColor3D color(0.f, 0.f, 0.f);
			mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			unsigned char data[] = {
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255,
				color.r * 255, color.g * 255, color.b * 255, 255
			};
			auto tex = Texture::createConstant(data, false, GL_UNSIGNED_BYTE);
			tex->setDesc(typeName);
			tex->setPath("Kd");
			textures.push_back(tex);
		}
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j]->getPath().data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					// a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{
				// if texture hasn't been loaded already, load it
				auto tex = Texture::createFromFile(str.C_Str(), directory, false, GL_UNSIGNED_BYTE);
				tex->setDesc(typeName);
				tex->setPath(str.C_Str());
				textures.push_back(tex);
				textures_loaded.push_back(tex);
				// store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}

	Material Model::loadMaterial(aiMaterial* mat)
	{
		Material material;
		aiColor3D color(0.f, 0.f, 0.f);
		float shininess;

		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material.Kd = glm::vec3(color.r, color.b, color.g);

		mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material.Ka = glm::vec3(color.r, color.b, color.g);

		mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material.Ks = glm::vec3(color.r, color.b, color.g);

		mat->Get(AI_MATKEY_SHININESS, shininess);
		material.Shininess = shininess;

		return material;
	}
}
