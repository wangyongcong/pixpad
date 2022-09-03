#pragma once
#include "engine.h"

namespace wyc
{
	extern WYCAPI HINSTANCE gModuleInstance;
	extern WYCAPI HINSTANCE gAppInstance;

#define GetModuleInstance() (wyc::gModuleInstance != NULL ? wyc::gModuleInstance : wyc::gAppInstance);

}
