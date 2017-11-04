#include <functional>
#include <strstream>
#include "test_line.h"
#include "test_box.h"
#include "test_texture.h"
#include "test_mipmap.h"
#define WYC_SHELLCMD_IMPLEMENTATION
#include "shellcmd.h"
#include "log.h"


std::unordered_map<std::string, std::function<CTest*()>> g_test_suit =
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

		std::unordered_map<std::string, std::function<CTest*()>> g_test_suit =
		{
			{ "line", &CTestLine::create },
			{ "box", &CTestBox::create },
			{ "texture", &CTestTexture::create },
			{ "mipmap", &CTestMipmap::create },
		};
	}

	virtual bool process(const po::variables_map &args) override
	{
		if (args.count("help")) {
			show_help();
			return true;
		}
		if (args["list"].as<bool>()) {
			for (auto &it : g_test_suit) {
				log_info("- %s", it.first.c_str());
			}
			return true;
		}
		if (!args.count("name")) {
			log_error("test name is not specified.");
			return false;
		}
		const std::string &test_name = args["name"].as<std::string>();
		auto it = g_test_suit.find(test_name.c_str());
		if (it == g_test_suit.end()) {
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


EXPORT_API wyc::IShellCommand** get_command_list(int &count)
{
	static CShellCmdTest s_cmd_test;
	static wyc::IShellCommand* s_cmd_lst[] = {
		&s_cmd_test,
	};
	count = sizeof(s_cmd_lst) / sizeof(void*);
	return s_cmd_lst;
}
