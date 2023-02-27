#include "windows_platform.h"

namespace wyc
{
	WYCAPI HINSTANCE g_module_instance = NULL;
	WYCAPI HINSTANCE g_app_instance = NULL;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,	// handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpReserved)	// reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		wyc::g_module_instance = hinstDLL;
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}