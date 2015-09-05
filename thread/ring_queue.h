#pragma once

#include <cassert>
#include <atomic>
#include <vector>

#include "platform_info.h"

#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete;			\
	void operator=(const TypeName&) = delete;	\
	TypeName(TypeName&&) = delete;				\
	void operator=(TypeName&&) = delete

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

		bool try_dequeue(T &ret)
		{
			auto write_pos = m_write_pos.load(std::memory_order_acquire);
			auto read_pos = m_read_pos.load(std::memory_order_relaxed);
			if (read_pos == write_pos)
			{
				// queue is empty
				return false;
			}
			auto i = read_pos & m_mask;
			ret = std::move(m_buffer[i]);
			m_buffer[i].~T();
			m_read_pos.store(read_pos + 1, std::memory_order_release);
			return true;
		}

		class enqueue_cursor
		{
		public:
			enqueue_cursor(ring_queue *q, size_t beg, size_t end)
				: m_queue(q)
				, m_beg(beg)
				, m_end(end)
			{
			}
			enqueue_cursor(const enqueue_cursor& other) = default;
			enqueue_cursor& operator = (const enqueue_cursor& other) = default;
			enqueue_cursor(enqueue_cursor&& other) = default;
			enqueue_cursor& operator = (enqueue_cursor&& other) = default;
			inline operator bool() const {
				return m_beg != m_end;
			}
			template<class ...Args>
			inline void set(Args&&... args)
			{
				assert(m_beg != m_end);
				new (&m_queue->m_buffer[m_beg & m_queue->m_mask]) T(std::forward<Args>(args)...);
				m_queue->publish(++m_beg);
			}
		private:
			ring_queue * m_queue;
			size_t m_beg, m_end;
		};

		enqueue_cursor batch_enqueue()
		{
			auto beg = m_write_pos.load(std::memory_order_relaxed);
			auto end = m_read_pos.load(std::memory_order_acquire) + m_size;
			return enqueue_cursor(this, beg, end);
		}

		bool batch_dequeue(std::vector<T> &ret)
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
				m_buffer[i].~T();
				++read_pos;
			}
			m_read_pos.store(read_pos, std::memory_order_release);
			return true;
		}

#ifdef _DEBUG
		void check_alignment()
		{
			std::cout << "this: " << this << std::endl;
			std::cout << "  m_buffer: " << &(this->m_buffer) << " offset=" << offsetof(ring_queue, m_buffer) << std::endl;
			std::cout << "  m_write_pos: " << &(this->m_write_pos) << " offset=" << offsetof(ring_queue, m_write_pos) << std::endl;
			std::cout << "  m_read_pos: " << &(this->m_read_pos) << " offset=" << offsetof(ring_queue, m_read_pos) << std::endl;
		}
#endif

	private:
		friend class enqueue_cursor;
		inline void publish(size_t pos)
		{
			m_write_pos.store(pos, std::memory_order_release);
		}

		T * const m_buffer;
		size_t m_size, m_mask;

		char _cache_line_padding1[CACHE_LINE_SIZE - sizeof(size_t) * 2 - sizeof(T*)];

		// producer
		std::atomic<size_t> m_write_pos;

		char _cache_line_padding2[CACHE_LINE_SIZE - sizeof(decltype(m_write_pos))];

		// consumer
		std::atomic<size_t> m_read_pos;


		DISALLOW_COPY_MOVE_AND_ASSIGN(ring_queue);
	};

} // namespace wyc

#include "unitest.h"
UNIT_TEST(ring_queue)