#include <atomic>
#include <memory>
#include <functional>
#include <strstream>
#define WYC_SHELLCMD_IMPLEMENTATION
#include "shellcmd.h"
#include "stb_log.h"
#include "test.h"

ENABLE_TEST(CTestLine)
ENABLE_TEST(CTestBox)
ENABLE_TEST(CTestTexture)
ENABLE_TEST(CTestMipmap)
ENABLE_TEST(CTestDepth)
ENABLE_TEST(CTestWireframe)
ENABLE_TEST(CTestLambert)
ENABLE_TEST(CTestRasterizer)

std::unordered_map<std::string, std::function<CTest*()>> g_test_suit =
{
	{ "line", &CREATE_TEST(CTestLine)},
	{ "box", &CREATE_TEST(CTestBox)},
	{ "texture", &CREATE_TEST(CTestTexture)},
	{ "mipmap", &CREATE_TEST(CTestMipmap)},
	{ "depth", &CREATE_TEST(CTestDepth)},
	{ "wireframe", &CREATE_TEST(CTestWireframe)},
	{ "lambert", &CREATE_TEST(CTestLambert)},
	{ "rasterizer", &CREATE_TEST(CTestRasterizer)},
};

class CTestTask;
static std::shared_ptr<CTestTask> g_task;

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

	inline bool is_done() const 
	{
		return m_is_done.load();
	}

	inline const CTest* get_test() const
	{
		return m_test;
	}

private:
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
	}

	virtual bool process(const po::variables_map &args) override
	{
		if (args.count("help")) {
			show_help();
			return true;
		}
		if (args["list"].as<bool>()) {
			for (auto &it : g_test_suit) {
				log_info("- %s", it.first);
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
			log_error("test is not found: %s", test_name);
			return false;
		}
		if (g_task) {
			log_error("previous test is still running, please be patient");
			return false;
		}
		log_info("start test [%s]...", test_name);
		CTest *test = it->second();
		if(!test->init(args)) {
			log_error("test initialize fail");
			return false;
		}
		auto task = std::make_shared<CTestTask>(test);
		g_task = task;
		std::async([task] {
			task->start();
		});
		return true;
	}

	virtual void show_help() const override
	{
		std::stringstream ss;
		ss << m_desc << std::endl;
		ss << "usage:" << std::endl;
		ss << "  [1] run test: test {name} [-o path]" << std::endl;
		ss << "  [2] list tests: test -l" << std::endl;
		ss << "options:" << std::endl;
		ss << m_opt;
		log_info(ss.str());
	}
};

#ifndef testbed_EXPORTS

wyc::IShellCommand* get_test_command()
{
	static CShellCmdTest s_cmd_test;
	return &s_cmd_test;
}

#else

EXPORT_API wyc::IShellCommand** get_command_list(int &count)
{
	static CShellCmdTest s_cmd_test;
	static wyc::IShellCommand* s_cmd_lst[] = {
		&s_cmd_test,
	};
	count = sizeof(s_cmd_lst) / sizeof(void*);
	return s_cmd_lst;
}

// 0 - no task
// 1 - task is running
// 2 - task is finished
EXPORT_API int get_task_state()
{
	if (!g_task)
		return 0;
	if (g_task->is_done())
		return 2;
	return 1;
}

EXPORT_API int get_task_result(const void **buf, unsigned &width, unsigned &height, unsigned &pitch)
{
	if (!g_task || !g_task->is_done())
		return -1;
	const CTest *test = g_task->get_test();
	*buf = test->get_color_buf(width, height, pitch);
	return 0;
}

EXPORT_API void clear_task()
{
	g_task = nullptr;
}

#endif // testbed_EXPORTS
