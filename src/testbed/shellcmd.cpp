#include "shellcmd.h"
#include <unordered_map>
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

namespace wyc
{
	class CShellCommand : public IShellCommand
	{
	public:
		CShellCommand(const char* name, const char *desc)
			: m_name(name)
			, m_desc(desc)
			, m_opt("")
			, m_pos_opt()
		{
		}

		virtual const std::string& name() const override {
			return m_name;
		}

		virtual const std::string& description() const override {
			return m_desc;
		}

		virtual bool execute(int argc, char *argv[]) override
		{
			po::variables_map args_table;
			if (!parse(argc, argv, args_table))
				return false;
			return process(args_table);
		}

		virtual bool execute(const std::string &cmdline) {
			po::variables_map args;
			if (!parse(cmdline, args))
				return false;
			return process(args);
		}

		virtual bool process(const po::variables_map &args)
		{
			if (args.count("help")) {
				show_help();
			}
			return true;
		}

		virtual void show_help() const
		{
			std::stringstream ss;
			ss << m_desc << std::endl << m_opt;
			log_info(ss.str().c_str());
		}

	protected:
		std::string m_name;
		std::string m_desc;
		po::options_description m_opt;
		po::positional_options_description m_pos_opt;

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
	};

	class CShellCmdTest : public CShellCommand
	{
	public:
		CShellCmdTest()
			: CShellCommand("test", "sparrow renderer test")
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

} // namespace

EXPORT_API wyc::IShellCommand* get_cmd_test()
{
	static wyc::CShellCmdTest s_test_cmd;
	return &s_test_cmd;
}

