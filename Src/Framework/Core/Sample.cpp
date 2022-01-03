#include "stdafx.h"
#include "Sample.h"
#include "Utils/StringUtils.h"
#include <sstream>
#include <fstream>
#include "Utils/Threading.h"
#include "imgui/imgui.h"

namespace Desperado
{
	IFramework* gpFramework = nullptr;

	void Sample::handleWindowSizeChange()
	{
		// Tell the device to resize the swap chain
		auto winSize = mpWindow->getClientAreaSize();

		// Tell the GUI the swap-chain size changed
		if (mpGui) mpGui->onWindowResize(winSize.x, winSize.y);
	}

	void Sample::handleRenderFrame()
	{
		renderFrame();
	}

	void Sample::handleKeyboardEvent(const KeyboardEvent& keyEvent)
	{
		if (mSuppressInput)
		{
			if (keyEvent.key == KeyboardEvent::Key::Escape) mpWindow->shutdown();
			return;
		}

		if (keyEvent.type == KeyboardEvent::Type::KeyPressed) mPressedKeys.insert(keyEvent.key);
		else if (keyEvent.type == KeyboardEvent::Type::KeyReleased) mPressedKeys.erase(keyEvent.key);

		//if (mShowUI && mpGui->onKeyboardEvent(keyEvent)) return;
		if (mpRenderer && mpRenderer->onKeyEvent(keyEvent)) return;

		// Consume system messages first
		if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
		{
			if (keyEvent.mods.isShiftDown && keyEvent.key == KeyboardEvent::Key::F12)
			{
				
			}
			else if (keyEvent.mods.isCtrlDown)
			{
				switch (keyEvent.key)
				{
				case KeyboardEvent::Key::Pause:
				case KeyboardEvent::Key::Space:
					mRendererPaused = !mRendererPaused;
					break;
				default:
					break;
				}
			}
			else if (!keyEvent.mods.isAltDown && !keyEvent.mods.isCtrlDown && !keyEvent.mods.isShiftDown)
			{
				switch (keyEvent.key)
				{
				case KeyboardEvent::Key::F12:
					mCaptureScreen = true;
					break;
				case KeyboardEvent::Key::V:
					mVsyncOn = !mVsyncOn;
					mFrameRate.reset();
					mClock.setTime(0);
					break;
				case KeyboardEvent::Key::F2:
					toggleUI(!mShowUI);
					break;
				case KeyboardEvent::Key::Escape:
					mpWindow->shutdown();
					break;
				case KeyboardEvent::Key::Pause:
				case KeyboardEvent::Key::Space:
					mClock.isPaused() ? mClock.play() : mClock.pause();
					break;
				default:
					break;
				}
			}
		}
	}

	void Sample::handleMouseEvent(const MouseEvent& mouseEvent)
	{
		if (mSuppressInput) return;
		//if (mShowUI && mpGui->onMouseEvent(mouseEvent)) return;
		if (mpRenderer && mpRenderer->onMouseEvent(mouseEvent)) return;
	}

	// Sample functions
	Sample::~Sample()
	{
		mpRenderer.reset();

		Clock::shutdown();
		Threading::shutdown();
		mpGui.reset();
		mpTargetFBO.reset();
	}

	void Sample::run(const SampleConfig& config, IRenderer::UniquePtr& pRenderer, uint32_t argc, char** argv)
	{
		Sample s(pRenderer);
		try
		{
			s.runInternal(config, argc, argv);
		}
		catch (const std::exception& e)
		{
			std::cout << ("Caught exception:\n\n" + std::string(e.what()) +
				"\n\nEnable breaking on exceptions in the debugger to get a full stack trace.") << std::endl;
		}
	}

	void Sample::runInternal(const SampleConfig& config, uint32_t argc, char** argv)
	{
		gpFramework = this;

		//Threading::start();

		mSuppressInput = config.suppressInput;
		mShowUI = config.showUI;
		mClock.setTimeScale(config.timeScale);
		if (config.pauseTime) mClock.pause();

		// Create the window
		mpWindow = Window::create(config.windowDesc, this);
		if (mpWindow == nullptr)
		{
			std::cout << ("Failed to create device and window") << std::endl;
			return;
		}

		Clock::start();

		// Init the UI
		initUI();

		// Load and run
		mpRenderer->onLoad(getRenderContext());

		mFrameRate.reset();
		mpWindow->msgLoop();

		mpRenderer->onShutdown();
		mpRenderer = nullptr;
	}

	void screenSizeUI(Gui::Widgets& widget, uint2 screenDims)
	{
		static const uint2 resolutions[] =
		{
			{0, 0},
			{1280, 720},
			{1920, 1080},
			{1920, 1200},
			{2560, 1440},
			{3840, 2160},
		};

		static const auto initDropDown = [](const uint2 resolutions[], uint32_t count) -> Gui::DropdownList
		{
			Gui::DropdownList list;
			for (uint32_t i = 0; i < count; i++)
			{
				list.push_back({i, std::to_string(resolutions[i].x) + "x" + std::to_string(resolutions[i].y)});
			}
			list[0] = {0, "Custom"};
			return list;
		};

		auto initDropDownVal = [](const uint2 resolutions[], uint32_t count, uint2 screenDims)
		{
			for (uint32_t i = 0; i < count; i++)
			{
				if (screenDims == resolutions[i]) return i;
			}
			return 0u;
		};

		static const Gui::DropdownList dropdownList = initDropDown(resolutions, (uint32_t)arraysize(resolutions));
		uint32_t currentVal = initDropDownVal(resolutions, (uint32_t)arraysize(resolutions), screenDims);

		widget.var("Screen Resolution", screenDims);
	}

	void Sample::renderGlobalUI(Gui* pGui)
	{

	}

	void Sample::renderUI()
	{
		if (mShowUI)
		{
			mpGui->beginFrame();

			if (mShowUI) mpRenderer->onGuiRender(mpGui.get());
			
			//mpGui->render(getRenderContext(), gpDevice->getSwapChainFbo(), (float)mFrameRate.getLastFrameTime());
		}
	}

	void Sample::renderFrame()
	{
		// Check clock exit condition
		if (mClock.shouldExit())
		{
		}

		mClock.tick();
		mFrameRate.newFrame();

		{
			if (!mRendererPaused)
			{
				RenderContext* pRenderContext = getRenderContext();
				mpRenderer->onFrameRender(pRenderContext, mpTargetFBO, mpWindow, mClock.getDelta());
			}
		}

		//renderUI();

		if (mCaptureScreen) captureScreen();
	}

	std::string Sample::captureScreen(const std::string explicitFilename, const std::string explicitOutputDirectory)
	{
		mCaptureScreen = false;

		std::string filename = explicitFilename;
		std::string outputDirectory = explicitOutputDirectory;

		std::string pngFile;

		return pngFile;
	}

	void Sample::initUI()
	{
		
	}

	bool Sample::isKeyPressed(const KeyboardEvent::Key& key)
	{
		return mPressedKeys.find(key) != mPressedKeys.cend();
	}

	SampleConfig Sample::getConfig()
	{
		SampleConfig c;
		
		c.windowDesc = mpWindow->getDesc();
		c.timeScale = (float)mClock.getTimeScale();
		c.pauseTime = mClock.isPaused();
		c.showUI = mShowUI;
		return c;
	}
}
