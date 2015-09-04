#include "folly_queue.h"
#include <cassert>
#include <iostream>
#include <thread>

UNIT_TEST_BEG(folly_queue)

#include "common.h"

void test()
{
	std::cout << "Test folly queue..." << std::endl;

	folly::ProducerConsumerQueue<message> queue(QUEUE_SIZE);

	bool start = false;

	std::thread producer([&]{
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