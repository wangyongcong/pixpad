#include "windows_application.h"

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	WindowsApplication::WindowsApplication(const wchar_t* appName)
		: GameApplication(appName)
	{

	}
}
