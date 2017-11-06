#include <atomic>
#include <memory>
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

class CTestTask
{
public:
	CTestTask(CTest *test)
		: m_test(test)
		, m_is_done(false)
	{

	}

	~CTestTask()
	{
		if (m_test) {
			delete m_test;
			m_test = nullptr;
		}
	}

	void start()
	{
		m_test->run();
		m_is_done.store(true);
	}

	inline bool is_task_done() const 
	{
		return m_is_done.load();
	}

	void get_result()
	{

	}

private:
	typedef std::pair<CTest*, std::atomic_bool> task_t;
	typedef std::shared_ptr<task_t> task_ptr;
	CTest *m_test;
	std::atomic_bool m_is_done;

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
		auto &test_name = args["name"].as<std::string>();
		auto it = g_test_suit.find(test_name.c_str());
		if (it == g_test_suit.end()) {
			log_error("test is not found: %s", test_name.c_str());
			return false;
		}
		if (m_task) {
			log_error("previous test is still running, please be patient");
			return false;
		}
		log_info("start test [%s]...", test_name.c_str());
		CTest *test = it->second();
		test->init(args);
		auto task = std::make_shared<CTestTask>(test);
		std::async([task] {
			task->start();
		});
		m_task = task;
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

private:
	std::shared_ptr<CTestTask> m_task;
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
