#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Utils/Timing/FrameRate.h"
#include "Utils/UI/Gui.h"
#include "Utils/Video/VideoEncoderUI.h"
#include <set>
#include <optional>

namespace Desperado
{
    /** Bootstrapper class for Falcor
        Call Sample::run() to start the sample.
        The render loop will then call the user's Renderer object
    */
    class dlldecl Sample : public Window::ICallbacks, public IFramework
    {
    public:
        /** Entry-point to Sample. User should call this to start processing.
            \param[in] config Requested sample configuration.
            \param[in] pRenderer The user's renderer. The Sample takes ownership of the renderer.
            \param[in] argc Optional. The number of strings in `argv`.
            \param[in] argv Optional. The command line arguments.
            Note that when running a Windows application (with WinMain()), the command line arguments will be retrieved and parsed even if argc and argv are nullptr.
        */
        static void run(const SampleConfig& config, IRenderer::UniquePtr& pRenderer, uint32_t argc = 0, char** argv = nullptr);

        virtual ~Sample();

    protected:
        /************************************************************************/
        /* Callback inherited from ICallbacks                                   */
        /************************************************************************/
        RenderContext* getRenderContext() override { return nullptr; }
        Fbo::SharedPtr getTargetFbo() override { return mpTargetFBO; }
        Window* getWindow() override { return mpWindow.get(); }
        Clock& getGlobalClock() override { return mClock; }
        FrameRate& getFrameRate() override { return mFrameRate; }
        void renderFrame() override;
        bool isKeyPressed(const KeyboardEvent::Key& key) override;
        void toggleUI(bool showUI) override { mShowUI = showUI; }
        bool isUiEnabled() override { return mShowUI; }
        void pauseRenderer(bool pause) override { mRendererPaused = pause; }
        bool isRendererPaused() override { return mRendererPaused; }
        std::string captureScreen(const std::string explicitFilename = "", const std::string explicitOutputDirectory = "") override;
        void shutdown() override { if (mpWindow) { mpWindow->shutdown(); } }
        SampleConfig getConfig() override;
        void renderGlobalUI(Gui* pGui) override;
        void toggleVsync(bool on) override { mVsyncOn = on; }
        bool isVsyncEnabled() override { return mVsyncOn; }

        /** Internal data structures
        */
        Gui::UniquePtr mpGui;                               ///< Main sample GUI
        Fbo::SharedPtr mpTargetFBO;                         ///< The FBO available to renderers
        bool mRendererPaused = false;                       ///< Freezes the renderer
        Window::SharedPtr mpWindow;                         ///< The application's window

        void handleWindowSizeChange() override;
        void handleRenderFrame() override;
        void handleKeyboardEvent(const KeyboardEvent& keyEvent) override;
        void handleMouseEvent(const MouseEvent& mouseEvent) override;

        // Private functions
        void initUI();

        void renderUI();

        void runInternal(const SampleConfig& config, uint32_t argc, char** argv);

        bool mSuppressInput = false;
        bool mVsyncOn = false;
        bool mShowUI = true;
        bool mCaptureScreen = false;
        FrameRate mFrameRate;
        Clock mClock;

        IRenderer::UniquePtr mpRenderer;

        std::set<KeyboardEvent::Key> mPressedKeys;

        Sample(IRenderer::UniquePtr& pRenderer) : mpRenderer(std::move(pRenderer)) {}
        Sample(const Sample&) = delete;
        Sample& operator=(const Sample&) = delete;
    };
};
