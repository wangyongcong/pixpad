#include <stdlib.h>
#include "unitest.h"
#include "log.h"
#include "render_command.h"

wyc::CLogger *g_log = nullptr;

int main()
{
	g_log = new wyc::CDebugLogger();

	RUN_TEST(render_command);

	::system("pause");
}