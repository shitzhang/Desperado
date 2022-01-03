#pragma once
#include "Desperado.h"

using namespace Desperado;

class SVGF : public IRenderer
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

    Texture::SharedPtr mpOut;

    Shader::SharedPtr mpSVGFGbufferShader;

    Fbo::SharedPtr mpGbufferFbo;

	Texture::SharedPtr mpPosTex;
	Texture::SharedPtr mpLinearZTex;
	Texture::SharedPtr mpNormalTex;
	Texture::SharedPtr mpPosNormalFwidthTex;
	Texture::SharedPtr mpAlbedoTex;
	Texture::SharedPtr mpMotionTex;
	Texture::SharedPtr mpMeshIDTex;
	Texture::SharedPtr mpEmissionTex;

	FullScreenPass::SharedPtr mpScreenDisplayPass;
	SVGFPass::SharedPtr mpSVGFPass;

	Texture::SharedPtr mpColorTex;
	Texture::SharedPtr mpDirectColorTex;
	Texture::SharedPtr mpIndirectColorTex;

	//test ray tracing
	Texture::SharedPtr mpOutputBufTex;
	Texture::SharedPtr mpOutputDirectBufTex;
	Texture::SharedPtr mpOutputIndirectBufTex;
	Texture::SharedPtr mpOutputTex;

	OptixContext::UniquePtr mpOptixContext;
    Shader::SharedPtr mpScreenShader;

    void setPerFrameVars(const Fbo* pTargetFbo);
    void renderRT();
    void processInput(GLFWwindow* window, double deltaTime);
    void loadScene();
};
