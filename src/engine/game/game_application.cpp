#include "engine_pch.h"
#include "engine.h"
#include <algorithm>
#include <filesystem>

#include "game_application.h"
#include "game_instance.h"
#include "common/log_macros.h"
#include "common/memory.h"

#ifdef PLATFORM_WINDOWS
#include "platform/windows/windows_application.h"
#include "platform/windows/windows_window.h"
using ApplicationClass = wyc::WindowsApplication;
using GameWindowClass = wyc::WindowsWindow;
#endif

#ifdef RENDERER_DIRECT3D12
#include "renderer/d3d12/renderer_d3d12.h"
using RenderClass = wyc::RendererD3D12;
#endif

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	WYCAPI GameApplication* g_application = nullptr;

	bool GameApplication::create_application(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		MemAllocInit();
		g_application = wyc_new(ApplicationClass, appName);
		g_application->start_logger();
		if(!g_application->initialize(appName, windowWidth, windowHeight))
		{
			wyc_delete(g_application);
			return false;
		}
		Log("Application started");
		return true;
	}

	void GameApplication::destroy_application()
	{
		if(g_application)
		{
			wyc_delete(g_application);
			g_application = nullptr;
			Log("Application exit");
		}
		close_logger();
		MemAllocExit();
	}

	static int64_t gHighResTimerFrequency = 0;

	void init_time()
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

	int64_t get_time()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart * (int64_t)1e6 / gHighResTimerFrequency;
	}

	float get_time_since(int64_t &start)
	{
		int64_t end = get_time();
		float delta = (float)(end - start) / (float)1e6;
		start = end;
		return delta;
	}

	void GameApplication::quit_qame(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	GameApplication::GameApplication(const wchar_t* appName)
		: m_app_name(appName)
		, m_game_instance(nullptr)
		, m_window(nullptr)
		, m_renderer(nullptr)
	{

	}

	GameApplication::~GameApplication()
	{
		if (m_renderer)
		{
			m_renderer->release();
			wyc_delete(m_renderer);
			m_renderer = nullptr;
		}
		if(m_window)
		{
			wyc_delete(m_window);
			m_window = nullptr;
		}
	}

	void GameApplication::show_window(bool visible)
	{
		if(m_window)
		{
			m_window->set_visible(visible);
		}
	}

	void GameApplication::start_game(IGameInstance* pGame)
	{
		m_game_instance = pGame;
		init_time();
		m_game_instance->initialize();
		int64_t lastTime = get_time();
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			float deltaTime = get_time_since(lastTime);
			m_game_instance->tick(deltaTime);
			m_renderer->begin_frame();
			m_game_instance->draw(m_renderer);
			m_renderer->present();
		}
		m_renderer->close();
		m_game_instance->exit();
	}

	bool GameApplication::initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		m_window = wyc_new(GameWindowClass);
		if(!m_window->create(appName, windowWidth, windowHeight))
		{
			return false;
		}
		m_renderer = wyc_new(RenderClass);
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
		if(!m_renderer->initialize(m_window, config))
		{
			LogError("Fail to initialize renderer");
			return false;
		}
		m_window->set_visible(true);
		return true;
	}

	void GameApplication::start_logger()
	{
		auto path = std::filesystem::current_path();
		path /= "saved/logs";
		std::wstring logName = m_app_name + L".log";
		std::replace(logName.begin(), logName.end(), L' ', L'-');
		path /= logName.c_str();
		std::string ansi = path.generic_string();
		start_file_logger(ansi.c_str());
	}

} // namespace wyc
