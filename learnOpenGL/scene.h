#pragma once

#include "light.h"
#include "camera.h"
#include "model.h"
#include "mesh.h"

#include <vector>

class Scene {
public:
	Scene() {}
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Model>> models;
	std::vector<std::shared_ptr<Mesh>> meshes;

	std::shared_ptr<Camera> pCamera;

	void AddLight(std::shared_ptr<Light> l) {
		lights.push_back(l);
	};
	void AddModel(std::shared_ptr<Model> m) {
		models.push_back(m);
	};
	void AddMesh(std::shared_ptr<Mesh> mesh) {
		meshes.push_back(mesh);
	};

	void DrawLights() {
		for (int i = 0; i < lights.size(); i++) {
			auto light = lights[i];

			glm::mat4 modelMatrix(1.0f);
			glm::mat4 viewMatrix(1.0f);
			glm::mat4 projectionMatrix(1.0f);

			TRStransform trans = light->entity.transform;

			modelMatrix = glm::translate(modelMatrix, trans.translate);

			modelMatrix = glm::scale(modelMatrix, trans.scale);

			if (trans.rotateAngle) {
				modelMatrix = glm::rotate(modelMatrix, trans.rotateAngle, trans.rotateAxis);
			}
				

			viewMatrix = pCamera->GetViewMatrix();
			projectionMatrix = pCamera->GetPerspectiveMatrix();

			auto shader = light->shader;
			shader.use();
			shader.setMat4("uModelMatrix", modelMatrix);
			shader.setMat4("uViewMatrix", viewMatrix);
			shader.setMat4("uProjectionMatrix", projectionMatrix);

			lights[i]->Draw();
		}
	}

	void DrawModels(Shader& shader) {
		for (int i = 0; i < models.size(); i++) {
			auto model = models[i];

			glm::mat4 modelMatrix(1.0f);
			glm::mat4 viewMatrix(1.0f);
			glm::mat4 projectionMatrix(1.0f);

			TRStransform trans = model->transform;

			modelMatrix = glm::translate(modelMatrix, trans.translate);

			modelMatrix = glm::scale(modelMatrix, trans.scale);
			if (trans.rotateAngle) {
				modelMatrix = glm::rotate(modelMatrix, trans.rotateAngle, trans.rotateAxis);
			}


			viewMatrix = pCamera->GetViewMatrix();
			projectionMatrix = pCamera->GetPerspectiveMatrix();

			shader.use();
			shader.setMat4("uModelMatrix", modelMatrix);
			shader.setMat4("uViewMatrix", viewMatrix);
			shader.setMat4("uProjectionMatrix", projectionMatrix);

			models[i]->Draw(shader);
		}
	}

	void DrawMeshes(Shader& shader) {
		for (int i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];

			glm::mat4 modelMatrix(1.0f);
			glm::mat4 viewMatrix(1.0f);
			glm::mat4 projectionMatrix(1.0f);

			TRStransform trans = mesh->transform;

			modelMatrix = glm::translate(modelMatrix, trans.translate);

			modelMatrix = glm::scale(modelMatrix, trans.scale);

			if (trans.rotateAngle) {
				modelMatrix = glm::rotate(modelMatrix, glm::radians(trans.rotateAngle), trans.rotateAxis);
			}
		

			viewMatrix = pCamera->GetViewMatrix();
			projectionMatrix = pCamera->GetPerspectiveMatrix();


			shader.use();
			shader.setMat4("uModelMatrix", modelMatrix);
			shader.setMat4("uViewMatrix", viewMatrix);
			shader.setMat4("uProjectionMatrix", projectionMatrix);
			meshes[i]->Draw(shader);
		}
	}
};
