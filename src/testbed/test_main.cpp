#include <functional>
#include <strstream>
#include "log.h"
#include "shellcmd.h"
#include "test.h"

EXPORT_API wyc::IShellCommand* get_cmd_test();

#ifndef testbed_EXPORTS

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	auto *cmd = get_cmd_test();
	if (!cmd->execute(argc, argv))
		return 1;
	return 0;
}

#else

#if (defined _WIN32 || defined _WIN64)
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

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

