#pragma once
#include "engine.h"

namespace wyc
{
	extern WYCAPI HINSTANCE g_module_instance;
	extern WYCAPI HINSTANCE g_app_instance;

#define GetModuleInstance() (wyc::g_module_instance != NULL ? wyc::g_module_instance : wyc::g_app_instance);

}
