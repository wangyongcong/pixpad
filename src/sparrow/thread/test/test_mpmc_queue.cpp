#include <Windows.h>

#include "mpmc_queue.h"

#include <stdio.h>
#include <time.h>
#include <process.h>
#include <iostream>
#include <string>

UNIT_TEST_BEG(mpmc_queue)

size_t const thread_count = 4;
size_t const batch_size = 1;
size_t const iter_count = 2000000;

bool volatile g_start = 0;

typedef dmitry::mpmc_bounded_queue<int> queue_t;

unsigned __stdcall thread_func(void* ctx)
{
	queue_t& queue = *(queue_t*)ctx;
	int data;

	srand((unsigned)time(0) + GetCurrentThreadId());
	size_t pause = rand() % 1000;

	while (g_start == 0)
		SwitchToThread();

	for (size_t i = 0; i != pause; i += 1)
		_mm_pause();

	for (int iter = 0; iter != iter_count; ++iter)
	{
		for (size_t i = 0; i != batch_size; i += 1)
		{
			while (!queue.enqueue(i))
				SwitchToThread();
		}
		for (size_t i = 0; i != batch_size; i += 1)
		{
			while (!queue.dequeue(data))
				SwitchToThread();
		}
	}

	return 0;
}

void test()
{
	queue_t queue(1024);

	HANDLE threads[thread_count];
	for (int i = 0; i != thread_count; ++i)
	{
		threads[i] = (HANDLE)_beginthreadex(0, 0, thread_func, &queue, 0, 0);
	}

	Sleep(1);

	unsigned __int64 start = __rdtsc();
	g_start = 1;

	WaitForMultipleObjects(thread_count, threads, 1, INFINITE);

	unsigned __int64 end = __rdtsc();
	unsigned __int64 time = end - start;
	std::cout << "cycles/op=" << time / (batch_size * iter_count * 2 * thread_count) << std::endl;
}

UNIT_TEST_END