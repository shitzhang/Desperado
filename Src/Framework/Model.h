#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <iostream>
#include <vector>

using namespace std;

namespace Desperado {
	class Mesh;
	class Shader;
	class dlldecl Model
	{
	public:
		vector<std::shared_ptr<Texture>> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
		vector<std::shared_ptr<Mesh>> p_meshes;
		string directory;
		bool gammaCorrection;

		TRStransform transform;

		// constructor, expects a filepath to a 3D model.
		Model(string const& path, TRStransform transform = TRStransform(), bool gamma = false);

		// draws the model, and thus all its meshes
		void Draw(std::shared_ptr<Shader> pShader);

	private:
		// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
		void loadModel(string const& path);

		// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode* node, const aiScene* scene);

		std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);

		// checks all material textures of a given type and loads the textures if they're not loaded yet.
		// the required info is returned as a Texture struct.
		vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

		Material loadMaterial(aiMaterial* mat);
	};
}
#endif
