#include <iostream>
#include <atomic>

#include "test_sampler.h"

using namespace std;
using namespace test;

int main()
{
	printf("Testint triangle sampler...");
	test_triangle_sampler();
	printf("OK!\n");
	return 0;
}

