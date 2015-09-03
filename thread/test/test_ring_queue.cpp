#include "ring_queue.h"

#include <iostream>
#include <thread>

UNIT_TEST_BEG(ring_queue)

struct message {
	unsigned long m_id;
	unsigned long m_data;
	message(unsigned long id) : m_id(id), m_data(0) {}
};

void test()
{
	const unsigned long N = 1024 * 1024 * 2;

	wyc::ring_queue<message> queue(256);

	bool start = false;

	std::thread producer([&]{
		while (!start)
		{
			std::this_thread::yield();
		}
		for (unsigned long i = 0; i < N; ++i)
		{
			while (!queue.enqueue(i))
			{
				std::this_thread::yield();
			}
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
			if (!queue.dequeue(batch))
			{
				std::this_thread::yield();
			}
			if (!batch.empty())
			{
				for (auto iter : batch)
				{
					assert(msg == iter.m_id);
					++msg;
				}
				batch.clear();
			}
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

	std::cout << "PASS: ms/op=" << float(duration.count()) / N << "; cycle/op=" << (end - beg) / N << std::endl;
}

UNIT_TEST_END