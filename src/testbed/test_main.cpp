#include <unordered_map>
#include <functional>
#include "boost/program_options.hpp"
#include "log.h"
#include "test_line.h"
#include "test_box.h"

namespace po = boost::program_options;

std::unordered_map<std::string, std::function<CTest*()>> cmd_lst =
{
	{"line", &CTestLine::create},
	{"box", &CTestBox::create},
};

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	std::cout << "argc=" << argc << std::endl;
	using string_list = std::vector<std::string>;
	po::options_description desc("Testbed for Sparrow renderer: testbed name [-o output_file]");
	desc.add_options()
		("help", "show help message")
		("name", po::value<std::string>(), "test to execute")
		("out,o", po::value<std::string>(), "output file path")
		;
	po::positional_options_description pos_desc;
	pos_desc.add("name", 1);

	po::variables_map args_table;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), args_table);
	po::notify(args_table);

	if (args_table.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	if (!args_table.count("name")) {
		log_error("Test name is not specified.");
		return 1;
	}

	try {
		const std::string &test_name = args_table["name"].as<std::string>();
		std::string out_file;
		if (args_table.count("out")) {
			out_file = args_table["out"].as<std::string>();
		}
		auto it = cmd_lst.find(test_name.c_str());
		if (it == cmd_lst.end()) {
			log_error("Invalid command: %s", argv[1]);
			return 1;
		}
		log_info("Running [%s]...", test_name.c_str());
		CTest *test = it->second();
		test->init();
		test->run();
	}
	catch (const po::error &exp) {
		log_error("Command line argument error: %s", exp.what());
		return 1;
	}
	log_info("finish and exit");
    return 0;
}

