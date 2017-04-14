#include <unordered_map>
#include <functional>
#include <strstream>
#include "boost/program_options.hpp"
#include "log.h"
#include "test_line.h"
#include "test_box.h"
#include "test_texture.h"

namespace po = boost::program_options;

std::unordered_map<std::string, std::function<CTest*()>> cmd_lst =
{
	{"line", &CTestLine::create},
	{"box", &CTestBox::create},
	{"texture", &CTestTexture::create},
};

inline void show_help(const po::options_description &desc)
{
	std::stringstream ss;
	ss << desc;
	log_info(ss.str().c_str());
}

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	using string_list = std::vector<std::string>;
	po::options_description desc("Testbed for Sparrow renderer: testbed name [-o output_file]");
	desc.add_options()
		("help", "show help message")
		("name", po::value<std::string>(), "test to execute")
		("out,o", po::value<std::string>(), "output file path")
		("param,p", po::value<std::vector<std::string>>(), "render params, e.g -p wireframe -p color=0xFFFFFFFF")
		("width,w", po::value<unsigned>()->default_value(960), "image width")
		("height,h", po::value<unsigned>()->default_value(540), "image height")
		;
	po::positional_options_description pos_desc;
	pos_desc.add("name", 1);

	po::variables_map args_table;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), args_table);
		po::notify(args_table);

		if (args_table.count("help")) {
			show_help(desc);
			return 0;
		}

		if (!args_table.count("name")) {
			log_error("test name is not specified.");
			return 1;
		}

		const std::string &test_name = args_table["name"].as<std::string>();
		auto it = cmd_lst.find(test_name.c_str());
		if (it == cmd_lst.end()) {
			log_error("invalid command: %s", test_name.c_str());
			return 1;
		}
		log_info("running %s...", test_name.c_str());
		CTest *test = it->second();
		test->init(args_table);
		test->run();
	}
	catch (const po::error &exp) {
		log_error("command line argument error: %s", exp.what());
		show_help(desc);
		return 1;
	}
	catch(...) {
		log_error("ERROR");
		return 1;
	}
	log_info("finish and exit");
    return 0;
}

