#pragma once
#include "game_framework.h"

namespace wyc
{
	extern GAME_FRAMEWORK_API HINSTANCE gModuleInstance;
	extern GAME_FRAMEWORK_API HINSTANCE gAppInstance;

#define GetModuleInstance() (wyc::gModuleInstance != NULL ? wyc::gModuleInstance : wyc::gAppInstance);

}
