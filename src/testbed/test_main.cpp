#include <unordered_map>
#include <functional>
#include "log.h"
#include "test_line.h"
#include "test_box.h"

std::unordered_map<std::string, std::function<CTest*()>> cmd_lst =
{
	{"line", &CTestLine::create},
	{"box", &CTestBox::create},
};

int main(int argc, char *argv[])
{
	wyc::init_debug_log();
	CTest *test = nullptr;
	if (argc > 1)
	{
		auto it = cmd_lst.find(argv[1]);
		if (it != cmd_lst.end())
		{
			test = it->second();
			try {
				test->init(argc, argv);
				test->run();
			}
			catch (TCLAP::ArgException &e)
			{
				log_error("ERROR: %s", e.error().c_str());
				delete test;
				return -1;
			}
			catch (std::exception &e)
			{
				log_error("ERROR: %s", e.what());
				delete test;
				return -1;
			}
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
	if (test)
		delete test;
	log_info("finish and exit");
    return 0;
}

