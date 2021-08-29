#include "SceneDepthPass.h"

SceneDepthPass::SceneDepthPass(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) {
	m_pScene = scene;
	m_pCamera = camera;

	initInfo();
};
SceneDepthPass::SceneDepthPass(const std::string& vPassName, int vExcutionOrder, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) :IRenderPass(vPassName, vExcutionOrder) {
	m_pScene = scene;
	m_pCamera = camera;
	initInfo();
};

void SceneDepthPass::initInfo() {
	m_pShader = std::make_shared<Shader>("shader/sceneDepthShader/depthVertex.glsl", "shader/sceneDepthShader/depthFragment.glsl");
	auto light = m_pScene->directionalLights[0];
	light->pFbo = std::make_shared<FBO>(1, m_resolution);
	m_fbo = light->pFbo->getFboIdx();

};

void SceneDepthPass::updateInfo() {
	auto light = m_pScene->directionalLights[0];
	//m_fbo = light->fbo.fbo_idx;

	m_pShader->use();
	//glm::mat4 model(1.0f);



	//auto dLight = (DirectionalLight*)(light.get());

	glm::mat4 lightVP = light->CalcLightVP();

	m_pShader->setMat4("uLightVP", lightVP);
};

void SceneDepthPass::render() {
	updateInfo();
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_resolution, m_resolution);

	m_pShader->use();
	m_pScene->DrawModels(*m_pShader);
	m_pScene->DrawMeshes(*m_pShader);


	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}