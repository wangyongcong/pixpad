#include "disruptor.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <future>
#include <cinttypes>

UNIT_TEST_BEG(disruptor_queue)

#include "common.h"

struct Event
{
	alignas(64) int64_t index;
};

struct Result
{
	alignas(64) int64_t id;
	alignas(64) int64_t c1;
	alignas(64) int64_t c2;
};

void test()
{
	using namespace disruptor;
	
	constexpr int SIZE = 32;
	constexpr int COUNT = 4096;

	ring_buffer<Event, SIZE> buff;

	auto sw = std::make_shared<shared_write_cursor>("SW", SIZE);
	auto r1 = std::make_shared<read_cursor>("r1");
	auto r2 = std::make_shared<read_cursor>("r2");
	r1->follows(sw);
	r2->follows(sw);
	sw->follows(r1);
	sw->follows(r2);

	Result data[COUNT];
	memset(data, sizeof(Result)*COUNT, 0);

	std::promise<void> p_start, c_start;
	std::shared_future<void> p_wait_start(p_start.get_future());
	std::shared_future<void> c_wait_start(c_start.get_future());

	auto p1 = std::thread([&, p_wait_start] {
		p_wait_start.wait();
		printf("p1 start\n");
		for (int i = 0; i < COUNT / 2; ++i)
		{
			auto start = sw->claim(1);
			data[i].id = i + 1;
			buff.at(start).index = i;
			//printf("p1 publish %d\n", start);
			sw->publish_after(start, start - 1); 
		}
	});

	auto p2 = std::thread([&, p_wait_start] {
		p_wait_start.wait();
		printf("p2 start\n");
		for (int i = COUNT / 2; i < COUNT; ++i)
		{
			auto start = sw->claim(1);
			data[i].id = i + 1;
			buff.at(start).index = i;
			//printf("p2 publish %d\n", start);
			sw->publish_after(start, start - 1);
		}
	});

	auto c1 = std::thread([&, c_wait_start] {
		//c_wait_start.wait();
		p_wait_start.wait();
		printf("c1 start\n");
		auto pos = r1->begin();
		auto end = r1->end();
		int64_t i = 0;
		while (i < COUNT)
		{
			if (pos == end)
			{
				if(end > 0) {
					r1->publish(end - 1);
					//printf("c1 pub %" PRId64 "\n", end - 1);
					if (end == COUNT)
						break;
				}
				end = r1->wait_for(end);
				//printf("c1 wait end [%" PRId64 ", %" PRId64 ")\n", pos, end);
			}
			i = buff.at(pos).index;
			data[i].c1 = 1;
			//printf("c1: %d\n", data[i].id);
			++pos;
		}
	});

	auto c2 = std::thread([&, c_wait_start] {
		//c_wait_start.wait();
		p_wait_start.wait();
		printf("c2 start\n");
		auto pos = r2->begin();
		auto end = r2->end();
		int64_t i = 0;
		while (i < COUNT)
		{
			if (pos == end)
			{
				if (end > 0) {
					r2->publish(end - 1);
					//printf("c2 pub %" PRId64 "\n", end - 1);
					if (end == COUNT)
						break;
				}
				end = r2->wait_for(end);
				//printf("c2 wait end [%" PRId64 ", %" PRId64 ")\n", pos, end);
			}
			i = buff.at(pos).index;
			data[i].c2 = 1;
			//printf("c2: %d\n", data[i].id);
			++pos;
		}
	});

	printf("start producer...\n");
	p_start.set_value();
	printf("start consumer...\n");
	c_start.set_value();

	p1.join();
	printf("p1 end\n");
	p2.join();
	printf("p2 end\n");
	c1.join();
	printf("c1 end\n");
	c2.join();
	printf("c2 end\n");

	for (int i = 0; i < COUNT; ++i)
	{
		assert(data[i].c1);
		assert(data[i].c2);
	}

	printf("test done!\n");
}

UNIT_TEST_END