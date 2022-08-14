#include "windows_application.h"
#include <filesystem>
#include "memory.h"
#include "game_instance.h"
#include "log_macros.h"
#include "windows_window.h"
#include "renderer_d3d12.h"

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	WindowsApplication::WindowsApplication(const wchar_t* appName)
		: GameApplication(appName)
	{

	}
}
