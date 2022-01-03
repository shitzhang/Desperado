#include "SVGF.h"


void SVGF::onGuiRender(Gui* pGui)
{
}

void SVGF::loadScene()
{
	mpCamera = make_shared<Camera>(glm::vec3(-500.0f, 700.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -10.0f);

	TRStransform sponzaTrans(glm::vec3(0.0, 0.0, 0.0));
	auto sponza = make_shared<Model>("../../../Model/sponza/sponza.obj", sponzaTrans);

	mpScene = std::make_shared<Scene>(mpCamera);
	mpScene->AddModel(sponza);
}

void SVGF::onLoad(RenderContext* pRenderContext)
{
	loadScene();

	mpScreenShader = std::make_shared<Shader>("Shader/framebuffers_screen.vs", "Shader/framebuffers_screen.fs");
	mpSVGFGbufferShader = std::make_shared<Shader>("Shader/SVGFGbuffer.vs", "Shader/SVGFGbuffer.fs");

	Fbo::Desc GbufferDesc;
	GbufferDesc.setColorTarget(0, GL_RGBA32F, GL_RGB); // gPosition
	GbufferDesc.setColorTarget(1, GL_RG32F, GL_RG); // gLinearZ
	GbufferDesc.setColorTarget(2, GL_RGBA32F, GL_RGB); // gNormal
	GbufferDesc.setColorTarget(3, GL_RG32F, GL_RG); // gPositionNormalFwidth
	GbufferDesc.setColorTarget(4, GL_RGBA32F, GL_RGB); // gAlbedo
	GbufferDesc.setColorTarget(5, GL_RG32F, GL_RG); // gMotion
	//GbufferDesc.setColorTarget(6, GL_R32F, GL_R);				// gMeshId
	GbufferDesc.setColorTarget(7, GL_RGB32F, GL_RGB); // gEmission
	GbufferDesc.setDepthStencilTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT);

	mpGbufferFbo = Fbo::create2D(SCR_WIDTH, SCR_HEIGHT, GbufferDesc);

	mpPosTex = mpGbufferFbo->getColorTexture(0);
	mpLinearZTex = mpGbufferFbo->getColorTexture(1);
	mpNormalTex = mpGbufferFbo->getColorTexture(2);
	mpPosNormalFwidthTex = mpGbufferFbo->getColorTexture(3);
	mpAlbedoTex = mpGbufferFbo->getColorTexture(4);
	mpMotionTex = mpGbufferFbo->getColorTexture(5);
	mpMeshIDTex = mpGbufferFbo->getColorTexture(6);
	mpEmissionTex = mpGbufferFbo->getColorTexture(7);

	mpScreenDisplayPass = FullScreenPass::create();

	mpColorTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	mpDirectColorTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	mpIndirectColorTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	//test ray tracing
	mpOutputBufTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	mpOutputDirectBufTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	mpOutputIndirectBufTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

	mpOutputTex = Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

	mpSVGFPass = SVGFPass::create();
	mpOptixContext = OptixContext::create("SVGF", "optixSVGF.cu", mpScene, mpCamera, SCR_WIDTH, SCR_HEIGHT);

	mpOptixContext->setContextTextureSampler(mpPosTex, "world_pos_tex");
	mpOptixContext->setContextTextureSampler(mpNormalTex, "world_normal_tex");
	mpOptixContext->setContextTextureSampler(mpAlbedoTex, "albedo_tex");

	mpOptixContext->validate();
}

void SVGF::setPerFrameVars(const Fbo* pTargetFbo)
{
}

void SVGF::renderRT()
{
	try
	{
		mpOptixContext->updateCamera();
		mpOptixContext->launch();
	}
	OPTIXUTIL_CATCH(mpOptixContext->getContext()->get())
}

void SVGF::processInput(GLFWwindow* window, double deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		mpCamera->ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		mpCamera->ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		mpCamera->ProcessKeyboard(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		mpCamera->ProcessKeyboard(RIGHT, deltaTime);
	}

}

void SVGF::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo, Window::SharedPtr pWindow, double deltaTime)
{
	processInput(pWindow->getGlfwHandle(), deltaTime);

	mpGbufferFbo->bind();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	mpSVGFGbufferShader->use();
	mpScene->DrawModels(mpSVGFGbufferShader);

	renderRT();

	auto colorBuffer = mpOptixContext->getBuffer("color_buffer");
	auto directColorBuffer = mpOptixContext->getBuffer("direct_color_buffer");
	auto indirectColorBuffer = mpOptixContext->getBuffer("indirect_color_buffer");

	auto outputBuffer = mpOptixContext->getBuffer("output_buffer");
	auto outputDirectBuffer = mpOptixContext->getBuffer("output_direct_buffer");
	auto outputIndirectBuffer = mpOptixContext->getBuffer("output_indirect_buffer");

	optixUtil::displayBufferGL(colorBuffer, mpColorTex);
	optixUtil::displayBufferGL(directColorBuffer, mpDirectColorTex);
	optixUtil::displayBufferGL(indirectColorBuffer, mpIndirectColorTex);

	optixUtil::displayBufferGL(outputBuffer, mpOutputBufTex);
	optixUtil::displayBufferGL(outputDirectBuffer, mpOutputDirectBufTex);
	optixUtil::displayBufferGL(outputIndirectBuffer, mpOutputIndirectBufTex);

	auto renderData = RenderData("SVGF", nullptr);
	renderData["gPosition"] = mpPosTex;
	renderData["gLinearZ"] = mpLinearZTex;
	renderData["gNormal"] = mpNormalTex;
	renderData["gPositionNormalFwidth"] = mpPosNormalFwidthTex;
	renderData["gAlbedo"] = mpAlbedoTex;
	renderData["gMotion"] = mpMotionTex;
	//renderData["gMeshId"] = mpMeshIDTex;
	renderData["gEmission"] = mpEmissionTex;

	renderData["color"] = mpColorTex;
	renderData["directColor"] = mpDirectColorTex;
	renderData["indirectColor"] = mpIndirectColorTex;
	renderData["output"] = mpOutputTex;

	mpSVGFPass->execute(renderData);

	mpScreenShader->use();
	mpScreenShader->setTexture2D("screenTexture", mpOutputTex);

	mpScreenDisplayPass->execute();
}

bool SVGF::onKeyEvent(const KeyboardEvent& keyEvent)
{
	return false;
}

bool SVGF::onMouseEvent(const MouseEvent& mouseEvent)
{
	mpCamera->ProcessMouseMovement(mouseEvent.screenOffset.x, mouseEvent.screenOffset.y);;
	mpCamera->ProcessMouseScroll(mouseEvent.wheelDelta.y);
	return true;
}

int main()
{
	SVGF::UniquePtr pRenderer = std::make_unique<SVGF>();

	SampleConfig config;
	config.windowDesc.title = "SVGF";
	config.windowDesc.resizableWindow = false;
	config.windowDesc.width = SCR_WIDTH;
	config.windowDesc.height = SCR_HEIGHT;

	Sample::run(config, pRenderer);

	return 0;
}
