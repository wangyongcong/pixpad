// sparrow.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#ifdef _DEBUG
	#pragma comment(lib, "libspw_staticd.lib")
#else
	#pragma comment(lib, "libspw_static.lib")
#endif

int main(int argc, char *argv[])
{
	printf("running sparrow...\n");
	for (int i = 0; i < argc; ++i)
		printf("[%d] %s\n", i, argv[i]);
    return 0;
}

