#pragma once
#include <string>
#include <cstdint>
#include "engine.h"
#include "game_window.h"
#include "renderer/renderer.h"
#include "common/common_macros.h"
#include "platform/platform.h"

namespace wyc
{
	class IGameInstance;

	class WYCAPI GameApplication
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(GameApplication)
	public:
		static bool create_application(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		static void destroy_application();

		explicit GameApplication(const wchar_t* appName);
		virtual ~GameApplication();

		void show_window(bool visible);
		void start_game(IGameInstance* pGame);
		void quit_qame(int exitCode);

		inline IGameWindow* get_window() const
		{
			return m_window;
		}
		inline IRenderer* get_renderer() const
		{
			return m_renderer;
		}

	protected:
		virtual bool initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		virtual void start_logger();

		std::wstring m_app_name;
		IGameInstance* m_game_instance;
		IGameWindow* m_window;
		IRenderer* m_renderer;
	};

	extern WYCAPI GameApplication* g_application;
} // namespace wyc

#ifdef PLATFORM_WINDOWS
#define APPLICATION_MAIN(GameInstanceClass) \
namespace wyc \
{\
	extern WYCAPI HINSTANCE g_app_instance;\
}\
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nShowCmd) \
{\
	wyc::g_app_instance = hInstance;\
	if(!wyc::GameApplication::create_application(L#GameInstanceClass, 1290, 720)) {\
		return 1;\
	}\
	GameInstanceClass game_instance;\
	g_application->start_game(&game_instance);\
	g_application->destroy_application();\
	return 0;\
}
#endif