#include <functional>
#include <strstream>
#include "log.h"
#define WYC_SHELLCMD_IMPLEMENTATION
#include "shellcmd.h"
#include "test_line.h"
#include "test_box.h"
#include "test_texture.h"
#include "test_mipmap.h"

std::unordered_map<std::string, std::function<CTest*()>> g_command_lst =
{
	{ "line", &CTestLine::create },
	{ "box", &CTestBox::create },
	{ "texture", &CTestTexture::create },
	{ "mipmap", &CTestMipmap::create },
};

class CShellCmdTest : public wyc::CShellCommand
{
public:
	CShellCmdTest()
		: wyc::CShellCommand("test", "Sparrow renderer test")
	{
		m_opt.add_options()
			("help", "show help message")
			("name", po::value<std::string>(), "test to execute")
			("out,o", po::value<std::string>(), "output file path")
			("param,p", po::value<std::vector<std::string>>(), "render params, e.g -p color=0xFFFFFF")
			("width,w", po::value<unsigned>()->default_value(960), "image width")
			("height,h", po::value<unsigned>()->default_value(540), "image height")
			("core,c", po::value<unsigned>()->default_value(0), "number of CPU core")
			("list,l", po::bool_switch()->default_value(false), "list available testing")
			;
		m_pos_opt.add("name", 1);
	}

	virtual bool process(const po::variables_map &args) override
	{
		if (args.count("help")) {
			show_help();
			return true;
		}
		if (args["list"].as<bool>()) {
			for (auto &it : g_command_lst) {
				log_info("- %s", it.first.c_str());
			}
			return true;
		}
		if (!args.count("name")) {
			log_error("test name is not specified.");
			return false;
		}
		const std::string &test_name = args["name"].as<std::string>();
		auto it = g_command_lst.find(test_name.c_str());
		if (it == g_command_lst.end()) {
			log_error("invalid command: %s", test_name.c_str());
			return false;
		}
		log_info("running %s...", test_name.c_str());
		CTest *test = it->second();
		test->init(args);
		test->run();
		log_info("finish");
		delete test;
		return true;
	}

	virtual void show_help()
	{
		std::stringstream ss;
		ss << m_desc << std::endl;
		ss << "usage:" << std::endl;
		ss << "  [1] run test: test {name} [-o path]" << std::endl;
		ss << "  [2] list tests: test -l" << std::endl;
		ss << "options:" << std::endl;
		ss << m_opt;
		log_info(ss.str().c_str());
	}
};


EXPORT_API wyc::IShellCommand* get_command()
{
	static CShellCmdTest s_test_cmd;
	return &s_test_cmd;
}

#ifndef testbed_EXPORTS

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	auto *cmd = get_command();
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

