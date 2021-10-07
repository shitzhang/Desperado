#pragma once

#include "mesh.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Desperado {
	class Fbo;
	class Light :public std::enable_shared_from_this<Light> {
	public:
		//virtual glm::mat4 CalcLightVP() = 0;
		void setLightMesh(std::shared_ptr<Mesh> p_mesh) {
			pEntity = p_mesh;
		}
		void setLightShader(std::shared_ptr<Shader> p_shader) {
			pShader = p_shader;
		}
		std::shared_ptr<Mesh> getLightMesh() {
			return pEntity;
		}
		std::shared_ptr<Shader> getLightShader() {
			return pShader;
		}
		void Draw() {
			if (pEntity.get() == NULL) {
				std::cout << "The light has no mesh" << endl;
				return;
			}
			if (pEntity.get() == NULL) {
				std::cout << "The light has no shader" << endl;
				return;
			}
			pEntity->Draw(pShader);
		}

		glm::vec3 emission;
		glm::vec3 lightPos;

		std::shared_ptr<Mesh> pEntity;
		std::shared_ptr<Shader> pShader;

		std::shared_ptr<Fbo> pFbo;
	};

	class DirectionalLight :public Light {
	public:
		DirectionalLight(glm::vec3 emis, glm::vec3 pos, glm::vec3 dir, glm::vec3 up) {
			emission = emis;
			lightPos = pos;
			lightDir = dir;
			lightUp = up;
		}

		glm::vec3 getShadingDirection() {
			return glm::normalize(-lightDir);
		}

		glm::mat4 getLightTrans() {
			glm::mat4 lightVP = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);
			glm::mat4 projectionMatrix = glm::mat4(1.0f);

			//View transform
			glm::vec3 focalPoint = lightPos + lightDir;
			viewMatrix = glm::lookAt(lightPos, focalPoint, lightUp);
			// Projection transform
			projectionMatrix = glm::ortho(left_plane, right_plane, bottom_plane, top_plane, near_plane, far_plane);

			return projectionMatrix * viewMatrix;

		}

		glm::vec3 lightDir;
		glm::vec3 lightUp;

		float left_plane;
		float right_plane;
		float top_plane;
		float bottom_plane;
		float near_plane;
		float far_plane;

	};

	class PointLight :public Light {
	public:
		PointLight(glm::vec3 emission, glm::vec3 pos) {
			emission = emission;
			lightPos = pos;
		}

		std::vector<glm::mat4> getPointLightTrans() {

			glm::mat4 shadowProj = glm::perspective(glm::radians(fov), (float)shadow_width / (float)shadow_height, near_plane, far_plane);
			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			return shadowTransforms;
		}

		float fov;
		unsigned int shadow_width;
		unsigned int shadow_height;
		float near_plane;
		float far_plane;
	};

	class ParallelogramLight :public Light {
	public:
		ParallelogramLight(glm::vec3 emission, glm::vec3 corner, glm::vec3 v1, glm::vec3 v2, glm::vec3 normal) {
			emission = emission;
			corner = corner;
			v1 = v1;
			v2 = v2;
			normal = normal;
		}

		virtual glm::mat4 CalcLightVP() {}

		glm::vec3 corner;
		glm::vec3 v1, v2;
		glm::vec3 normal;
	};
}