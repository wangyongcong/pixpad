#include <iostream>
#include <conio.h>
#include <thread>

#include "test_sampler.h"

using namespace std;
using namespace test;

int main()
{
	test_polygon_clipping();
	printf("Press any key to continue\n");
	while (!::_kbhit())
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return 0;
}

