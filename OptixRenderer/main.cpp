#include "stdafx.h"
#include<iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// camera
shared_ptr<Desperado::Camera> camera = make_shared<Desperado::Camera>(glm::vec3(.0f, 10.0f, .0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);            //限制了opengl版本为3.3，导致optix创建buffer出现问题，所以注释掉
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OptixRenderer", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSwapInterval(0);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	auto screenShader = std::make_shared<Desperado::Shader>("../shader/framebuffers_screen.vs", "../shader/framebuffers_screen.fs");

	Desperado::TRStransform cornellTrans(glm::vec3(0.0, 0.0, 0.0));
	Desperado::TRStransform sponzaTrans(glm::vec3(0.0, 0.0, 0.0));
	Desperado::TRStransform MarryTrans(glm::vec3(0.0, 0.0, 0.0));
	Desperado::TRStransform nanoTrans(glm::vec3(0.0, 0.0, 0.0));

	//Model cornell_box("model/cornell_box/CornellBox-Empty-CO.obj", cornellTrans);
	auto sponza = make_shared<Desperado::Model>("../model/sponza/sponza.obj", sponzaTrans);
	//Model Mary("model/Marry/Marry.obj", MarryTrans);
	//Model nanosuit("model/nanosuit/nanosuit.obj", nanoTrans);

	auto pScene = std::make_shared<Desperado::Scene>(camera);
	pScene->AddModel(sponza);

	auto SVGFGbufferShader = std::make_shared<Desperado::Shader>("../shader/SVGF/SVGFGbuffer.vs", "../shader/SVGF/SVGFGbuffer.fs");

	Desperado::Fbo::Desc GbufferDesc;
	GbufferDesc.setColorTarget(0, GL_RGBA32F, GL_RGBA);			// gPositionMeshId
	GbufferDesc.setColorTarget(1, GL_RG32F, GL_RG);				// gLinearZ
	GbufferDesc.setColorTarget(2, GL_RGB32F, GL_RGB);			// gNormal
	GbufferDesc.setColorTarget(3, GL_RG32F, GL_RG);			// gPositionNormalFwidth
	GbufferDesc.setColorTarget(4, GL_RGB32F, GL_RGB);			// gAlbedo
	GbufferDesc.setColorTarget(5, GL_RG32F, GL_RG);				// gMotion
	//GbufferDesc.setColorTarget(6, GL_R32F, GL_R);				// gMeshId
	GbufferDesc.setColorTarget(7, GL_RGB32F, GL_RGB);			// gEmission
	GbufferDesc.setDepthStencilTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT);
	auto GbufferFbo = Desperado::Fbo::create2D(SCR_WIDTH, SCR_HEIGHT, GbufferDesc);

	auto screenDisplayPass = Desperado::FullScreenPass::create();

	auto colorTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	auto directColorTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	auto indirectColorTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	//test ray tracing
	auto outputBufTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	auto outputDirectBufTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);
	auto outputIndirectBufTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

	auto outputTex = Desperado::Texture::create2D(SCR_WIDTH, SCR_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1, 0, nullptr);

	auto svgfPass = Desperado::SVGFPass::create();
	auto pOptixContext = Desperado::OptixContext::create("SVGF", "optixSVGF.cu", pScene, camera, SCR_WIDTH, SCR_HEIGHT);

	try {
		pOptixContext->validate();

		// render loop
		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			processInput(window);

			pOptixContext->updateCamera();
			pOptixContext->launch();
			//glBindFramebuffer(GL_FRAMEBUFFER, GbufferFbo->getId());
			GbufferFbo->bind();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);
			SVGFGbufferShader->use();
			pScene->DrawModels(SVGFGbufferShader);

			auto colorBuffer = pOptixContext->getBuffer("color_buffer");
			auto directColorBuffer = pOptixContext->getBuffer("direct_color_buffer");
			auto indirectColorBuffer = pOptixContext->getBuffer("indirect_color_buffer");

			/*auto outputBuffer = pOptixContext->getBuffer("output_buffer");
			auto outputDirectBuffer = pOptixContext->getBuffer("output_direct_buffer");
			auto outputIndirectBuffer = pOptixContext->getBuffer("output_indirect_buffer");*/

			optixUtil::displayBufferGL(colorBuffer, colorTex);
			optixUtil::displayBufferGL(directColorBuffer, directColorTex);
			optixUtil::displayBufferGL(indirectColorBuffer, indirectColorTex);

			/*optixUtil::displayBufferGL(outputBuffer, outputBufTex);
			optixUtil::displayBufferGL(outputDirectBuffer, outputDirectBufTex);
			optixUtil::displayBufferGL(outputIndirectBuffer, outputIndirectBufTex);*/

			auto posMeshIdTex = GbufferFbo->getColorTexture(0);
			auto linearZTex = GbufferFbo->getColorTexture(1);
			auto normalTex = GbufferFbo->getColorTexture(2);
			auto posNormalFwidthTex = GbufferFbo->getColorTexture(3);
			auto albedoTex = GbufferFbo->getColorTexture(4);
			auto motionTex = GbufferFbo->getColorTexture(5);
			//auto meshIDTex = GbufferFbo->getColorTexture(6);
			auto emissionTex = GbufferFbo->getColorTexture(7);

			auto renderData = Desperado::RenderData("SVGF", nullptr);
			renderData["gPositionMeshId"] = posMeshIdTex;
			renderData["gLinearZ"] = linearZTex;
			renderData["gNormal"] = normalTex;
			renderData["gPositionNormalFwidth"] = posNormalFwidthTex;
			renderData["gAlbedo"] = albedoTex;
			renderData["gMotion"] = motionTex;
			//renderData["gMeshId"] = meshIDTex;
			renderData["gEmission"] = emissionTex;

			renderData["color"] = colorTex;
			renderData["directColor"] = directColorTex;
			renderData["indirectColor"] = indirectColorTex;
			renderData["output"] = outputTex;

			svgfPass->execute(renderData);

			//gamma correction
			//glEnable(GL_FRAMEBUFFER_SRGB);

			//glActiveTexture(GL_TEXTURE0);
			screenShader->use();
			//screenShader->setInt("screenTexture", 0);
			screenShader->setTexture2D("screenTexture", outputTex);
			//albedoTex->bind();

			{
				static unsigned frame_count = 0;
				displayFps(frame_count++, window);
			}

			screenDisplayPass->execute();

			// 交换缓冲并查询IO事件
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		glfwTerminate();
	}
	OPTIXUTIL_CATCH(pOptixContext->getContext()->get())
		return 0;
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		//destroyContext();
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->ProcessKeyboard(Desperado::FORWARD, deltaTime);
		camera_changed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->ProcessKeyboard(Desperado::BACKWARD, deltaTime);
		camera_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->ProcessKeyboard(Desperado::LEFT, deltaTime);
		camera_changed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->ProcessKeyboard(Desperado::RIGHT, deltaTime);
		camera_changed = true;
	}

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	//SCR_WIDTH = width;
	//SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset);
	camera_changed = true;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(yoffset);
	camera_changed = true;
}




