// sparrow.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <unordered_map>
#include <functional>
#include "log.h"

#ifdef _DEBUG
	#pragma comment(lib, "libspw_staticd.lib")
	#pragma comment(lib, "Imath-2_2.lib")
	#pragma comment(lib, "IexMath-2_2.lib")
	#pragma comment(lib, "Iex-2_2.lib")
	#pragma comment(lib, "Half.lib")
	#pragma comment(lib, "libpng16_staticd.lib")
	#pragma comment(lib, "zlibstaticd.lib")
#else
	#pragma comment(lib, "libspw_static.lib")
#endif

wyc::CLogger *g_log = nullptr;

#define REG_MODULE(mod_name) extern void mod_name##_execute(int argc, char *argv[])
#define GET_MODULE(mod_name) (&mod_name##_execute)

REG_MODULE(test_line);
REG_MODULE(test_collada);

std::unordered_map<std::string, std::function<void(int, char*[])>> cmd_lst = 
{
	{ "test_line", GET_MODULE(test_line) },
	{ "test_collada", GET_MODULE(test_collada) },
};

int main(int argc, char *argv[])
{
	g_log = new wyc::CDebugLogger();
	if (argc > 1)
	{
		auto it = cmd_lst.find(argv[1]);
		if (it != cmd_lst.end())
		{
			it->second(argc, argv);
		}
		else
		{
			log_error("invalid command: %s", argv[1]);
		}
	}
	else
	{
		log_info("sparrow cmd [args...]");
	}
	log_info("finish and exit");
    return 0;
}

