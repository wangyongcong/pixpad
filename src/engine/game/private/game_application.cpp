#include "game_framework_pch.h"

#include <algorithm>
#include <filesystem>

#include "game_application.h"
#include "memory.h"
#include "game_instance.h"
#include "log_macros.h"

#ifdef PLATFORM_WINDOWS
#include "windows_application.h"
#include "windows_window.h"
using ApplicationClass = wyc::WindowsApplication;
using GameWindowClass = wyc::WindowsWindow;
#endif

#ifdef RENDERER_DIRECT3D12
#include "renderer_d3d12.h"
using RenderClass = wyc::RendererD3D12;
#endif

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	GAME_FRAMEWORK_API GameApplication* gApplication = nullptr;

	bool GameApplication::create_application(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		MemAllocInit();
		gApplication = tf_new(ApplicationClass, appName);
		gApplication->start_logger();
		if(!gApplication->initialize(appName, windowWidth, windowHeight))
		{
			tf_delete(gApplication);
			return false;
		}
		Log("Application started");
		return true;
	}

	void GameApplication::destroy_application()
	{
		if(gApplication)
		{
			tf_delete(gApplication);
			gApplication = nullptr;
			Log("Application exited");
		}
		close_logger();
		MemAllocExit();
	}

	static int64_t gHighResTimerFrequency = 0;

	void InitTime()
	{
		LARGE_INTEGER frequency;
		if (QueryPerformanceFrequency(&frequency))
		{
			gHighResTimerFrequency = frequency.QuadPart;
		}
		else
		{
			gHighResTimerFrequency = 1000LL;
		}
	}

	int64_t GetTime()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart * (int64_t)1e6 / gHighResTimerFrequency;
	}

	float GetTimeSince(int64_t &start)
	{
		int64_t end = GetTime();
		float delta = (float)(end - start) / (float)1e6;
		start = end;
		return delta;
	}

	void GameApplication::quit_qame(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	GameApplication::GameApplication(const wchar_t* appName)
		: mAppName(appName)
		, mpGameInstance(nullptr)
		, mpWindow(nullptr)
		, mpRenderer(nullptr)
	{

	}

	GameApplication::~GameApplication()
	{
		if (mpRenderer)
		{
			mpRenderer->release();
			tf_delete(mpRenderer);
			mpRenderer = nullptr;
		}
		if(mpWindow)
		{
			tf_delete(mpWindow);
			mpWindow = nullptr;
		}
	}

	void GameApplication::show_window(bool visible)
	{
		if(mpWindow)
		{
			mpWindow->set_visible(visible);
		}
	}

	void GameApplication::start_game(IGameInstance* pGame)
	{
		mpGameInstance = pGame;
		InitTime();
		mpGameInstance->initialize();
		int64_t lastTime = GetTime();
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			float deltaTime = GetTimeSince(lastTime);
			mpGameInstance->tick(deltaTime);
			mpRenderer->begin_frame();
			mpGameInstance->draw(mpRenderer);
			mpRenderer->present();
		}
		mpRenderer->close();
		mpGameInstance->exit();
	}

	bool GameApplication::initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		mpWindow = tf_new(GameWindowClass);
		if(!mpWindow->create(appName, windowWidth, windowHeight))
		{
			return false;
		}
		mpRenderer = tf_new(RenderClass);
		const RendererConfig config {
			3,
			1,
			TinyImageFormat_R8G8B8A8_SRGB,
			TinyImageFormat_D24_UNORM_S8_UINT,
#ifdef _DEBUG
			true,
#else
			false,
#endif
		};
		if(!mpRenderer->initialize(mpWindow, config))
		{
			LogError("Fail to initialize renderer");
			return false;
		}
		mpWindow->set_visible(true);
		return true;
	}

	void GameApplication::start_logger()
	{
		auto path = std::filesystem::current_path();
		path /= "Saved";
		path /= "Logs";
		std::wstring logName = mAppName + L".log";
		std::replace(logName.begin(), logName.end(), L' ', L'-');
		path /= logName.c_str();
		std::string ansi = path.generic_string();
		start_file_logger(ansi.c_str());
	}

} // namespace wyc
