#pragma once

#include "Light.h"

#include <vector>

namespace Desperado {
	class Camera;
	class Model;
	class Mesh;
	class dlldecl Scene {
	public:
		using SharedPtr = std::shared_ptr<Scene>;

		Scene(std::shared_ptr<Camera> pC);

		void SetCamera(std::shared_ptr<Camera> pC);

		void AddDirectionalLight(std::shared_ptr<DirectionalLight> l);
		void AddPointLight(std::shared_ptr<PointLight> l);
		void AddParallelogramLight(std::shared_ptr<ParallelogramLight> l);

		void AddModel(std::shared_ptr<Model> m);
		void AddMesh(std::shared_ptr<Mesh> mesh);
		std::vector<std::shared_ptr<Model>> getModels() const;
		std::vector<std::shared_ptr<Mesh>> getMeshes() const;

		void DrawLights();
		void DrawModels(std::shared_ptr<Shader> pShader);
		void DrawMeshes(std::shared_ptr<Shader> pShader);

	private:
		std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
		std::vector<std::shared_ptr<PointLight>> pointLights;
		std::vector<std::shared_ptr<ParallelogramLight>> parallelogramLights;
		std::vector<std::shared_ptr<Model>> models;
		std::vector<std::shared_ptr<Mesh>> meshes;

		std::shared_ptr<Camera> pCamera;

		//SVGF use
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		glm::mat4 preViewMatrix;
		glm::mat4 preProjectionMatrix;

		uint32_t frameNum = 0;
	};
}
