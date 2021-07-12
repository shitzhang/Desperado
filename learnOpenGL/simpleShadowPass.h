#pragma once

#include "pass.h"
#include "light.h"


class SimpleShadowPass :public IRenderPass {
public:
	SimpleShadowPass(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) {
		m_pScene = scene;
		m_pCamera = camera;

		initInfo();
	};
	SimpleShadowPass(const std::string& vPassName, int vExcutionOrder, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) :IRenderPass(vPassName, vExcutionOrder) {
		m_pScene = scene;
		m_pCamera = camera;
		initInfo();
	};

	virtual void initInfo() {
		m_pShader = std::make_shared<Shader>("shader/simpleShadowShader/simpleShadow.vs", "shader/simpleShadowShader/simpleShadow.fs");
		m_fbo = 0;
	};

	virtual void updateInfo() {
		auto light = m_pScene->lights[0];		

		m_pShader->use();	
		auto dLight = dynamic_pointer_cast<DirectionalLight>(light);
		glm::mat4 lightVP = dLight->CalcLightVP();
		m_pShader->setMat4("uLightVP", lightVP);

		unsigned int depthTex = light->pFbo->textures[0];
		
		m_pShader->setInt("uShadowMap", 10);
		glActiveTexture(GL_TEXTURE0 + 10);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		
	};

	virtual void render() {
		updateInfo();
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		m_pShader->use();
		m_pScene->DrawLights();
		m_pScene->DrawModels(*m_pShader);
		m_pScene->DrawMeshes(*m_pShader);


		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
private:
};
