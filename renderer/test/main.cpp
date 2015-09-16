#include <stdlib.h>
#include "unitest.h"
#include "log.h"
#include "render_command.h"

wyc::xlogger *g_log = nullptr;

int main()
{
	g_log = new wyc::debug_logger();

	RUN_TEST(render_command);

	::system("pause");
}