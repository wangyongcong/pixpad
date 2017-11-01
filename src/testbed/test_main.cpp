#include <unordered_map>
#include <functional>
#include <strstream>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include "log.h"
#include "test_line.h"
#include "test_box.h"
#include "test_texture.h"
#include "test_mipmap.h"

namespace po = boost::program_options;

std::unordered_map<std::string, std::function<CTest*()>> g_command_lst =
{
	{ "line", &CTestLine::create },
	{ "box", &CTestBox::create },
	{ "texture", &CTestTexture::create },
	{ "mipmap", &CTestMipmap::create },
};

class CCommand
{
public:
	CCommand(const char *cmd_desc)
		: m_opt(cmd_desc)
		, m_pos_opt()
	{
	}

	void show_help()
	{
		std::stringstream ss;
		ss << m_opt;
		log_info(ss.str().c_str());
	}

	bool parse(int argc, char *argv[], po::variables_map &args_table)
	{
		try {
			po::store(po::command_line_parser(argc, argv).options(m_opt).positional(m_pos_opt).run(), args_table);
			po::notify(args_table);
		}
		catch (const po::error &exp) {
			log_error(exp.what());
			show_help();
			return false;
		}
		return true;
	}

	bool parse(const std::string &cmd_line, po::variables_map &args_table)
	{
		std::vector<std::string> result;
		boost::escaped_list_separator<char> seperator("\\", "= ", "\"\'");
		boost::tokenizer<decltype(seperator)> tokens(cmd_line, seperator);
		for (auto &tok : tokens)
		{
			if (!tok.empty())
				result.push_back(tok);
		}
		try {
			po::store(po::command_line_parser(result).options(m_opt).positional(m_pos_opt).run(), args_table);
			po::notify(args_table);
		}
		catch (const po::error &exp) {
			log_error(exp.what());
			show_help();
			return false;
		}
		return true;
	}

protected:
	po::options_description m_opt;
	po::positional_options_description m_pos_opt;
};

class CCommandTest : public CCommand
{
public:
	CCommandTest()
		: CCommand("Sparrow renderer test\nusage:\n[1] run test: test {name} [-o path]\n[2] list tests: test -l\noptions")
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
};

void run_test(const po::variables_map &args)
{
	if (args["list"].as<bool>()) {
		for (auto &it : g_command_lst) {
			log_info("- %s", it.first.c_str());
		}
		return;
	}
	if (!args.count("name")) {
		log_error("test name is not specified.");
		return;
	}
	const std::string &test_name = args["name"].as<std::string>();
	auto it = g_command_lst.find(test_name.c_str());
	if (it == g_command_lst.end()) {
		log_error("invalid command: %s", test_name.c_str());
		return;
	}
	log_info("running %s...", test_name.c_str());
	CTest *test = it->second();
	test->init(args);
	test->run();
	log_info("finish");
}

#ifndef testbed_EXPORTS

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	CCommandTest test_cmd;
	po::variables_map args_table;
	if (!s_test_cmd.parse(argc, argv, args_table))
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

EXPORT_API bool testbed(const std::string &cmd_line)
{
	static CCommandTest s_test_cmd;
	po::variables_map args_table;
	if (!s_test_cmd.parse(cmd_line, args_table))
		return false;
	if (args_table.count("help")) {
		s_test_cmd.show_help();
		return true;
	}
	run_test(args_table);
	return true;
}

#endif

#endif // testbed_EXPORTS

