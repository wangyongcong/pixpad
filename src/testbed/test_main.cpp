#include "test.h"
#include "shellcmd.h"

#ifndef testbed_EXPORTS

wyc::IShellCommand* get_test_command();

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	auto *cmd = get_test_command();
	if (!cmd->execute(argc, argv))
		return 1;
	return 0;
}

#else

#if (defined _WIN32 || defined _WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

EXPORT_API void set_logger(wyc::ILogger *logger)
{
	LOGGER_SET(logger);
}

#endif

#endif // testbed_EXPORTS

