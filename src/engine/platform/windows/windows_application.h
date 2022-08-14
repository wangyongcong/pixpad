#pragma once
#include "game_application.h"

namespace wyc
{
	class WindowsApplication: public GameApplication
	{
	public:
		explicit WindowsApplication(const wchar_t* appName);

	};
}
