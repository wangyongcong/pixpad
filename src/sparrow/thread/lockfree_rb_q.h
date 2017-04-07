/**
* Implementation of Naive and Lock-free ring-buffer queues and
* performance and verification tests.
*
* Build with (g++ version must be >= 4.5.0):
* $ g++ -Wall -std=c++0x -Wl,--no-as-needed -O2 -D DCACHE1_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE` lockfree_rb_q.cc -lpthread
*
* I verified the program with g++ 4.5.3, 4.6.1, 4.6.3 and 4.8.1.
*
* Use -std=c++11 instead of -std=c++0x for g++ 4.8.
*
* Copyright (C) 2012-2013 Alexander Krizhanovsky (ak@natsys-lab.com).
*
* This file is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
* See http://www.gnu.org/licenses/lgpl.html .
*/

// Adapted from: https://github.com/natsys/blog/blob/master/lockfree_rb_q.cc
// More info: http://natsys-lab.blogspot.hk/2013/05/lock-free-multi-producer-multi-consumer.html

#ifdef _MSC_VER
#if !defined(_M_IX86) && !defined(_M_X64)
	#pragma message("The program is developed for x86-64 architecture only.")
#endif
#else
#ifndef __x86_64__
#warning "The program is developed for x86-64 architecture only."
#endif
#endif
#if !defined(DCACHE1_LINESIZE) || !DCACHE1_LINESIZE
#ifdef DCACHE1_LINESIZE
#undef DCACHE1_LINESIZE
#endif
#define DCACHE1_LINESIZE 64
#endif
#define ____cacheline_aligned	__attribute__((aligned(DCACHE1_LINESIZE)))

#include <malloc.h>
#include <algorithm>
#include <atomic>
#include <cassert>

#ifdef _MSC_VER
#include "platform_info.h"
inline size_t getpagesize()
{
	return wyc::page_size();
}

inline void* align_alloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

inline void align_free(void *ptr)
{
	_aligned_free(ptr);
}

#else
#define align_alloc memalign
#define align_free free
#endif

#define QUEUE_SIZE	(32 * 1024)

/*
* ------------------------------------------------------------------------
* Lock-free N-producers M-consumers ring-buffer queue.
* ABA problem safe.
*
* This implementation is bit complicated, so possibly it has sense to use
* classic list-based queues. See:
* 1. D.Fober, Y.Orlarey, S.Letz, "Lock-Free Techniques for Concurrent
*    Access to Shared Ojects"
* 2. M.M.Michael, M.L.Scott, "Simple, Fast and Practical Non-Blocking and
*    Blocking Concurrent Queue Algorithms"
* 3. E.Ladan-Mozes, N.Shavit, "An Optimistic Approach to Lock-Free FIFO Queues"
*
* See also implementation of N-producers M-consumers FIFO and
* 1-producer 1-consumer ring-buffer from Tim Blechmann:
*	http://tim.klingt.org/boost_lockfree/
*	git://tim.klingt.org/boost_lockfree.git
*
* See See Intel 64 and IA-32 Architectures Software Developer's Manual,
* Volume 3, Chapter 8.2 Memory Ordering for x86 memory ordering guarantees.
* ------------------------------------------------------------------------
*/
static thread_local size_t __thr_id;

/**
* @return continous thread IDs starting from 0 as opposed to pthread_self().
*/
inline size_t
thr_id()
{
	return __thr_id;
}

inline void
set_thr_id(size_t id)
{
	__thr_id = id;
}

template<class T,
	decltype(thr_id) ThrId = thr_id,
	unsigned long Q_SIZE = QUEUE_SIZE>
class LockFreeQueue {
private:
	static const unsigned long Q_MASK = Q_SIZE - 1;

	struct ThrPos {
		unsigned long head, tail;
	};

public:
	LockFreeQueue(size_t n_producers, size_t n_consumers)
		: n_producers_(n_producers),
		n_consumers_(n_consumers),
		head_(0),
		tail_(0),
		last_head_(0),
		last_tail_(0)
	{
		auto n = std::max(n_consumers_, n_producers_);
		thr_p_ = (ThrPos *)::align_alloc(getpagesize(), sizeof(ThrPos) * n);
		assert(thr_p_);
		// Set per thread tail and head to ULONG_MAX.
		::memset((void *)thr_p_, 0xFF, sizeof(ThrPos) * n);

		ptr_array_ = (T **)::align_alloc(getpagesize(),
			Q_SIZE * sizeof(void *));
		assert(ptr_array_);
	}

	~LockFreeQueue()
	{
		::align_free(ptr_array_);
		::align_free(thr_p_);
	}

	ThrPos&
		thr_pos() const
	{
		assert(ThrId() < std::max(n_consumers_, n_producers_));
		return thr_p_[ThrId()];
	}

	void enqueue(T *ptr)
	{
		/*
		* Request next place to push.
		*
		* Second assignemnt is atomic only for head shift, so there is
		* a time window in which thr_p_[tid].head = ULONG_MAX, and
		* head could be shifted significantly by other threads,
		* so pop() will set last_head_ to head.
		* After that thr_p_[tid].head is setted to old head value
		* (which is stored in local CPU register) and written by @ptr.
		*
		* First assignment guaranties that pop() sees values for
		* head and thr_p_[tid].head not greater that they will be
		* after the second assignment with head shift.
		*
		* Loads and stores are not reordered with locked instructions,
		* se we don't need a memory barrier here.
		*/
		thr_pos().head = head_;
#ifdef _MSC_VER
		thr_pos().head = head_.fetch_add(1);
#else
		thr_pos().head = __sync_fetch_and_add(&head_, 1);
#endif
		/*
		* We do not know when a consumer uses the pop()'ed pointer,
		* se we can not overwrite it and have to wait the lowest tail.
		*/
#ifdef _MSC_VER
		while (thr_pos().head >= last_tail_ + Q_SIZE)
#else
		while (__builtin_expect(thr_pos().head >= last_tail_ + Q_SIZE, 0))
#endif
		{
			unsigned long min = tail_;

			// Update the last_tail_.
			for (size_t i = 0; i < n_consumers_; ++i) {
				auto tmp_t = thr_p_[i].tail;

#ifdef _MSC_VER
				// Compiler read-write fence (by wyc)
				std::atomic_signal_fence(std::memory_order_seq_cst);
#else
				// Force compiler to use tmp_h exactly once.
				asm volatile("" ::: "memory");
#endif

				if (tmp_t < min)
					min = tmp_t;
			}
			last_tail_ = min;

			if (thr_pos().head < last_tail_ + Q_SIZE)
				break;
			std::this_thread::yield();
		}

		ptr_array_[thr_pos().head & Q_MASK] = ptr;

		// Allow consumers eat the item.
		thr_pos().head = ULONG_MAX;
	}

	T * dequeue()
	{
		/*
		* Request next place from which to pop.
		* See comments for push().
		*
		* Loads and stores are not reordered with locked instructions,
		* se we don't need a memory barrier here.
		*/
		thr_pos().tail = tail_;
#ifdef _MSC_VER
		thr_pos().tail = tail_.fetch_add(1);
#else
		thr_pos().tail = __sync_fetch_and_add(&tail_, 1);
#endif

		/*
		* tid'th place in ptr_array_ is reserved by the thread -
		* this place shall never be rewritten by push() and
		* last_tail_ at push() is a guarantee.
		* last_head_ guaraties that no any consumer eats the item
		* before producer reserved the position writes to it.
		*/
#ifdef _MSC_VER
		while (thr_pos().tail >= last_head_)
#else
		while (__builtin_expect(thr_pos().tail >= last_head_, 0))
#endif
		{
			unsigned long min = head_;

			// Update the last_head_.
			for (size_t i = 0; i < n_producers_; ++i) {
				auto tmp_h = thr_p_[i].head;

#ifdef _MSC_VER
				// Compiler read-write fence (by wyc)
				std::atomic_signal_fence(std::memory_order_seq_cst);
#else
				// Force compiler to use tmp_h exactly once.
				asm volatile("" ::: "memory");
#endif

				if (tmp_h < min)
					min = tmp_h;
			}
			last_head_ = min;

			if (thr_pos().tail < last_head_)
				break;
			std::this_thread::yield();
		}

		T *ret = ptr_array_[thr_pos().tail & Q_MASK];
		// Allow producers rewrite the slot.
		thr_pos().tail = ULONG_MAX;
		return ret;
	}

private:
	/*
	* The most hot members are cacheline aligned to avoid
	* False Sharing.
	*/

	const size_t n_producers_, n_consumers_;
	// currently free position (next to insert)
	std::atomic<unsigned long> head_;
	// current tail, next to pop
	std::atomic<unsigned long> tail_;
	// last not-processed producer's pointer
	std::atomic<unsigned long> last_head_;
	// last not-processed consumer's pointer
	std::atomic<unsigned long> last_tail_;
	ThrPos *thr_p_;
	T **ptr_array_;
};

#include "unitest.h"

UNIT_TEST(lockfree_rb_q)

