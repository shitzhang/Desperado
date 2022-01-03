#pragma once
#include "Window.h"
#include "Utils/UI/UserInput.h"

namespace Desperado
{
	class RenderContext;
	class Clock;
    class FrameRate;
    class Fbo;
    class Gui;

    /** Sample configuration
    */
    struct SampleConfig
    {
        Window::Desc windowDesc;                 ///< Controls window creation
        bool suppressInput = false;              ///< Suppress all keyboard and mouse input (other than escape to terminate)
        bool showMessageBoxOnError = true;       ///< Show message box on framework/API errors.
        float timeScale = 1.0f;                  ///< A scaling factor for the time elapsed between frames
        bool pauseTime = false;                  ///< Control whether or not to start the clock when the sample start running
        bool showUI = true;                      ///< Show the UI
    };

    class IFramework
    {
    public:
        /** Get the render-context for the current frame. This might change each frame.
        */
        virtual RenderContext* getRenderContext() = 0;

        /** Get the current FBO.
        */
        virtual std::shared_ptr<Fbo> getTargetFbo() = 0;

        /** Get the window.
        */
        virtual Window* getWindow() = 0;

        /** Get the global Clock object.
        */
        virtual Clock& getGlobalClock() = 0;

        /** Get the global FrameRate object.
        */
        virtual FrameRate& getFrameRate() = 0;

        /** Render a frame.
        */
        virtual void renderFrame() = 0;

        /** Check if a key is pressed.
        */
        virtual bool isKeyPressed(const KeyboardEvent::Key& key) = 0;

        /** Show/hide the UI.
        */
        virtual void toggleUI(bool showUI) = 0;

        /** Show/hide the UI.
        */
        virtual bool isUiEnabled() = 0;

        /** Takes and outputs a screenshot.
        */
        virtual std::string captureScreen(const std::string explicitFilename = "", const std::string explicitOutputDirectory = "") = 0;

        /** Shutdown the app.
        */
        virtual void shutdown() = 0;

        /** Pause/resume the renderer. The GUI will still be rendered.
        */
        virtual void pauseRenderer(bool pause) = 0;

        /** Check if the renderer running.
        */
        virtual bool isRendererPaused() = 0;

        /** Get the current configuration.
        */
        virtual SampleConfig getConfig() = 0;

        /** Render the global UI. You'll can open a GUI window yourself before calling it.
        */
        virtual void renderGlobalUI(Gui* pGui) = 0;

        /** Set VSYNC.
        */
        virtual void toggleVsync(bool on) = 0;

        /** Get the VSYNC state.
        */
        virtual bool isVsyncEnabled() = 0;
    };

    dlldecl extern IFramework* gpFramework;

    class IRenderer
    {
    public:
        using UniquePtr = std::unique_ptr<IRenderer>;
        IRenderer() = default;
        virtual ~IRenderer() {};

        /** Called once right after context creation.
        */
        virtual void onLoad(RenderContext* pRenderContext) {}

        /** Called on each frame render.
        */
        virtual void onFrameRender(RenderContext* pRenderContext, const std::shared_ptr<Fbo>& pTargetFbo, Window::SharedPtr pWindow, double deltaTime) {}

        /** Called right before the context is destroyed.
        */
        virtual void onShutdown() {}

        /** Called every time a key event occurred.
        \param[in] keyEvent The keyboard event
        \return true if the event was consumed by the callback, otherwise false
        */
        virtual bool onKeyEvent(const KeyboardEvent& keyEvent) { return false; }

        /** Called every time a mouse event occurred.
        \param[in] mouseEvent The mouse event
        \return true if the event was consumed by the callback, otherwise false
        */
        virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }

        /** Called after onFrameRender().
        It is highly recommended to use onGuiRender() exclusively for GUI handling. onGuiRender() will not be called when the GUI is hidden, which should help reduce CPU overhead.
        You could also ignore this and render the GUI directly in your onFrameRender() function, but that is discouraged.
        */
        virtual void onGuiRender(Gui* pGui) {};

        // Deleted copy operators (copy a pointer type!)
        IRenderer(const IRenderer&) = delete;
        IRenderer& operator=(const IRenderer&) = delete;
    };
}
