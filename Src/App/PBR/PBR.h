#pragma once
#include "Desperado.h"

using namespace Desperado;

class PBR : public IRenderer
{
public:
	void onLoad(RenderContext* pRenderContext) override;
	void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo, Window::SharedPtr pWindow, double deltaTime) override;
	bool onKeyEvent(const KeyboardEvent& keyEvent) override;
	bool onMouseEvent(const MouseEvent& mouseEvent) override;
	void onGuiRender(Gui* pGui) override;

private:

	Scene::SharedPtr mpScene;

	Camera::SharedPtr mpCamera;

	

	void setPerFrameVars(const Fbo* pTargetFbo);
	void renderRT();
	void processInput(GLFWwindow* window, double deltaTime);
	void loadScene();
};

