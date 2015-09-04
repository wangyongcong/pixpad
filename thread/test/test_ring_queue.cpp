#include "ring_queue.h"
#include <cassert>
#include <iostream>
#include <thread>

UNIT_TEST_BEG(ring_queue)

#include "common.h"

// use batch enqueue and dequeue
#define BATCH_LOAD 

void test()
{
	std::cout << "Test wyc ring queue..." << std::endl;

	wyc::ring_queue<message> queue(QUEUE_SIZE);

#ifdef _DEBUG
	queue.check_alignment();
#endif

	bool start = false;

#ifdef BATCH_LOAD
	std::thread producer([&]{
		while (!start)
		{
			std::this_thread::yield();
		}
		auto cursor = queue.batch_enqueue();
		for (unsigned long i = 0; i < N; ++i)
		{
			while(!cursor)
			{ 
				std::this_thread::yield();
				cursor = queue.batch_enqueue();
			}
			cursor.set(i, i);
		}
	});

	std::thread consumer([&] {
		std::vector<message> batch;
		unsigned long msg = 0;
		while (!start)
		{
			std::this_thread::yield();
		}
		while (msg != N)
		{
			if (!queue.batch_dequeue(batch))
			{
				std::this_thread::yield();
			}
			if (!batch.empty())
			{
				for (auto &iter : batch)
				{
					assert(msg == iter.m_id);
					busy(iter);
					++msg;
				}
				batch.clear();
			}
		}
	});
#else
	std::thread producer([&] {
		while (!start)
		{
			std::this_thread::yield();
		}
		for (unsigned long i = 0; i < N; ++i)
		{
			while (!queue.enqueue(i, i))
			{
				std::this_thread::yield();
			}
		}
	});

	std::thread consumer([&] {
		message msg;
		unsigned long next_id = 0;
		while (!start)
		{
			std::this_thread::yield();
		}
		while (next_id != N)
		{
			while (!queue.try_dequeue(msg))
			{
				std::this_thread::yield();
			}
			assert(next_id == msg.m_id);
			busy(msg);
			++next_id;
		}
	});
#endif  // BATCH_LOAD

	auto t0 = std::chrono::high_resolution_clock::now();
	unsigned __int64 beg = __rdtsc();

	start = true;

	producer.join();
	consumer.join();

	unsigned __int64 end = __rdtsc();
	auto t1 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

	std::cout << "Pass: retuls=0x" << std::hex << std::uppercase << g_result.m_sum << std::dec << std::endl;
	std::cout << "Stat: ms/op=" << float(duration.count()) / N << "; cycle/op=" << (end - beg) / N << std::endl;
}

UNIT_TEST_END