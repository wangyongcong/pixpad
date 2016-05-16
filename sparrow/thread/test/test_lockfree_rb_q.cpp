#include "lockfree_rb_q.h"

#include <iostream>
#include <thread>

/*
* ------------------------------------------------------------------------
*	Tests for lock-free queues
* ------------------------------------------------------------------------
*/

UNIT_TEST_BEG(lockfree_rb_q)

static const auto N = QUEUE_SIZE * 32;
static const auto CONSUMERS = 2;
static const auto PRODUCERS = 2;

typedef unsigned char	q_type;

static const q_type X_EMPTY = 0; // the address skipped by producers
static const q_type X_MISSED = 255; // the address skipped by consumers
q_type x[N * PRODUCERS];
std::atomic<int> n(0);

template<class Q>
struct Worker {
	Worker(Q *q, size_t id = 0)
		: q_(q),
		thr_id_(id)
	{}

	Q *q_;
	size_t thr_id_;
};

template<class Q>
struct Producer : public Worker<Q> {
	Producer(Q *q, size_t id)
		: Worker<Q>(q, id)
	{}

	void operator()()
	{
		set_thr_id(Worker<Q>::thr_id_);

		for (auto i = thr_id(); i < N * PRODUCERS; i += PRODUCERS) {
			x[i] = X_MISSED;
			Worker<Q>::q_->enqueue(x + i);
		}
	}
};

template<class Q>
struct Consumer : public Worker<Q> {
	Consumer(Q *q, size_t id)
		: Worker<Q>(q, id)
	{}

	void operator()()
	{
		set_thr_id(Worker<Q>::thr_id_);

		while (n.fetch_add(1) < N * PRODUCERS) {
			q_type *v = Worker<Q>::q_->dequeue();
			assert(v);
			assert(*v == X_MISSED);
			*v = (q_type)(thr_id() + 1); // don't write zero
		}
	}
};

template<class Q>
void run_test(Q &&q)
{
	std::thread thr[PRODUCERS + CONSUMERS];

	n.store(0);
	::memset(x, X_EMPTY, N * sizeof(q_type) * PRODUCERS);

	auto tv0 = std::chrono::high_resolution_clock::now();

	// Run producers.
	for (auto i = 0; i < PRODUCERS; ++i)
		thr[i] = std::thread(Producer<Q>(&q, i));

	// sleep to wait the queue is full
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	/*
	* Run consumers.
	* Create consumers with the same thread IDs as producers.
	* The IDs are used for queue head and tail indexing only,
	* so we  care only about different IDs for threads of the same type.
	*/
	for (auto i = 0; i < CONSUMERS; ++i)
		thr[PRODUCERS + i] = std::thread(Consumer<Q>(&q, i));

	// Wait for all threads completion.
	for (auto i = 0; i < PRODUCERS + CONSUMERS; ++i)
		thr[i].join();

	auto tv1 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tv1 - tv0);
	std::cout << duration.count() << "ms" << std::endl;

	// Check data.
	auto res = 0;
	std::cout << "check X data..." << std::endl;
	for (auto i = 0; i < N * PRODUCERS; ++i) {
		if (x[i] == X_EMPTY) {
			std::cout << "empty " << i << std::endl;
			res = 1;
			break;
		}
		else if (x[i] == X_MISSED) {
			std::cout << "missed " << i << std::endl;
			res = 2;
			break;
		}
	}
	std::cout << (res ? "FAILED" : "Passed") << std::endl;
}

void test()
{

	LockFreeQueue<q_type> lf_q(PRODUCERS, CONSUMERS);
	run_test<LockFreeQueue<q_type>>(std::move(lf_q));

}

UNIT_TEST_END