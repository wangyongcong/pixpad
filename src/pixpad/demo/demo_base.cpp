#include "stb/stb_log.h"

// all demo process
void demo_one_triangle();

extern "C" {
	void demo_main()
	{
		start_logger();
		
		demo_one_triangle();
		
		close_logger();
	}
}
