#pragma once

#include "mesh.h"
#include "shader.h"
#include "global.h"

namespace Desperado {
	class Skybox {
	public:
		Skybox(Shader& shader) :entity(cube(TRStransform())), shader(shader) {

		}
		std::shared_ptr<Mesh> entity;
		Shader shader;
		unsigned int tex_id;
		void Draw() {
			tex_id = loadCubemap(skyboxFaces);
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			shader.use();
			shader.setInt("skybox", 0);
			//view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix

			//skyboxShader.setMat4("view", view);
			//skyboxShader.setMat4("projection", projection);
			// skybox cube

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

			entity->Draw(shader);

			glDepthFunc(GL_LESS); // set depth function back to default
		};
	};
}
