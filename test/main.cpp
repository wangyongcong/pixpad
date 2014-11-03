#include <iostream>
#include <conio.h>
#include <thread>

#include "test_sampler.h"

using namespace test;

int main()
{
	test_vector();
//	test_matrix();
//	test_homogeneous_clipping();
	printf("Press any key to continue\n");
	while (!::_kbhit())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}

