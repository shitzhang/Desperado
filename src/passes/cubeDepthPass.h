#pragma once
#pragma once

#include "pass.h"
#include "light.h"


class pointShadowDepthPass :public IRenderPass {
public:
	pointShadowDepthPass(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) {
		m_pScene = scene;
		m_pCamera = camera;

		initInfo();
	};
	pointShadowDepthPass(const std::string& vPassName, int vExcutionOrder, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) :IRenderPass(vPassName, vExcutionOrder) {
		m_pScene = scene;
		m_pCamera = camera;
		initInfo();
	};

	virtual void initInfo() {
		m_pShader = std::make_shared<Shader>("shader/pointShadow/pointShadowDepth.vs", "shader/pointShadow/pointShadowDepth.fs", "shader/pointShadow/pointShadowDepth.gs");
		auto light = m_pScene->pointLights[0];
		light->pFbo = make_shared<FBO>(AttachmentType::AttachmentType_CubeDepth, light->shadow_height, light->shadow_height);
		m_fbo = light->pFbo->getFboIdx();
	};

	virtual void updateInfo() {
		auto light = m_pScene->pointLights[0];

		m_pShader->use();
		
		vector<glm::mat4> shadowTransforms = light->getPointLightTrans();

		for (unsigned int i = 0; i < 6; ++i)
			m_pShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		m_pShader->setFloat("far_plane", light->far_plane);
		m_pShader->setVec3("lightPos", light->lightPos);

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
