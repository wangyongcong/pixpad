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
			return mpWindow;
		}
		inline IRenderer* get_renderer() const
		{
			return mpRenderer;
		}

	protected:
		virtual bool initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		virtual void start_logger();

		std::wstring mAppName;
		IGameInstance* mpGameInstance;
		IGameWindow* mpWindow;
		IRenderer* mpRenderer;
	};

	extern WYCAPI GameApplication* gApplication;
} // namespace wyc

#ifdef PLATFORM_WINDOWS
#define APPLICATION_MAIN(GameInstanceClass) \
namespace wyc \
{\
	extern WYCAPI HINSTANCE gAppInstance;\
}\
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nShowCmd) \
{\
	wyc::gAppInstance = hInstance;\
	if(!wyc::GameApplication::create_application(L#GameInstanceClass, 1290, 720)) {\
		return 1;\
	}\
	GameInstanceClass gameInstance;\
	gApplication->start_game(&gameInstance);\
	gApplication->destroy_application();\
	return 0;\
}
#endif