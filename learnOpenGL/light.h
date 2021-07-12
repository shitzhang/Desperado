#pragma once

#include "mesh.h"
#include "shader.h"
#include "FBO.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Light {
public:
	
	Light(glm::vec3 radiance ,glm::vec3 pos,Mesh &entity,Shader &shader):
		lightRadiance(radiance),lightPos(pos),entity(entity),shader(shader)
	{
		//fbo = FBO(1);
	}

	virtual glm::vec3 CalShadingDirection() = 0;

	glm::vec3 lightRadiance;
	glm::vec3 lightPos;

	Mesh entity;
	Shader shader;

	std::shared_ptr<FBO> pFbo;

	void Draw() {
		entity.Draw(shader);
	}

};

class DirectionalLight:public Light {
public:
	DirectionalLight(glm::vec3 radiance, glm::vec3 pos, glm::vec3 dir, glm::vec3 up, Mesh& entity, Shader& shader) :Light(radiance, pos, entity, shader) {
		lightDir = dir;
		lightUp = up;
	}
	glm::vec3 lightDir;
	glm::vec3 lightUp;
	
	virtual glm::vec3 CalShadingDirection() {
		return glm::normalize(-lightDir);
	}
	
	glm::mat4 CalcLightVP() {
		glm::mat4 lightVP = glm::mat4(1.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);

		//View transform
		glm::vec3 focalPoint = lightPos + lightDir;
		viewMatrix = glm::lookAt(lightPos, focalPoint, lightUp);
		// Projection transform
		projectionMatrix = glm::ortho(-120.0, 120.0, -80.0, 120.0, 1.0, 500.0);

		return projectionMatrix * viewMatrix;
		
	}

};