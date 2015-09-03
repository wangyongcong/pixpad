#pragma once

#include <cassert>
#include <atomic>
#include <vector>

#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;           \
  TypeName(const TypeName&&) = delete;          \
  void operator=(const TypeName&) = delete

namespace wyc
{
	// lock-free, single-producer, single-consumer, bounded ring queue
	template<class T>
	class ring_queue
	{
	public:
		ring_queue(size_t sz) :
			m_size(sz), 
			m_mask(sz - 1), 
			m_buffer(static_cast<T*>(std::malloc(sizeof(T) * sz))),
			m_read_pos(0), 
			m_write_pos(0)
		{
			assert(sz >= 2 && (sz & (sz - 1)) == 0);
		}

		~ring_queue()
		{
			auto read_pos = m_read_pos.load(std::memory_order_relaxed);
			auto end = m_write_pos.load(std::memory_order_relaxed);
			if (!std::is_trivially_destructible<T>::value) 
			{
				while (read_pos != end)
				{
					m_buffer[read_pos & m_mask].~T();
					++read_pos;
				}
			}
			std::free(m_buffer);
		}

		template<class ...Args>
		bool enqueue(Args&&... args)
		{
			auto read_pos = m_read_pos.load(std::memory_order_acquire);
			auto write_pos = m_write_pos.load(std::memory_order_relaxed);
			if (write_pos == read_pos + m_size)
			{
				// queue is full
				return false;
			}
			new (&m_buffer[write_pos & m_mask]) T(std::forward<Args>(args)...);
			m_write_pos.store(write_pos + 1, std::memory_order_release);
			return true;
		}

		bool dequeue(std::vector<T> &ret)
		{
			auto write_pos = m_write_pos.load(std::memory_order_acquire);
			auto read_pos = m_read_pos.load(std::memory_order_relaxed);
			if (read_pos == write_pos)
			{
				// queue is empty
				return false;
			}
			while (read_pos != write_pos)
			{
				auto i = read_pos & m_mask;
				ret.push_back(std::move(m_buffer[i]));
				if (!std::is_trivially_destructible<T>::value)
				{
					m_buffer[i].~T();
				}
				++read_pos;
			}
			m_read_pos.store(read_pos, std::memory_order_release);
			return true;
		}

	private:
		size_t m_size, m_mask;
		T * const m_buffer;
		
		// producer
		std::atomic<unsigned long> m_write_pos;

		// consumer
		std::atomic<unsigned long> m_read_pos;


		DISALLOW_COPY_MOVE_AND_ASSIGN(ring_queue);
	};

} // namespace wyc

#include "unitest.h"

UNIT_TEST(ring_queue)