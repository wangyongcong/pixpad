#pragma once
#include <thread>
#include <atomic>

namespace wyc
{
	// author: Guo Zhongming
	// https://www.zhihu.com/question/55764216/answer/146139092
	class CSpinLock
	{
	public:
		CSpinLock() : m_lock(false) {}
		void lock() {
			do {
				while (m_lock == true) {
					_mm_pause(); // pause, about 12ns
					if (!m_lock.load(std::memory_order_relaxed)) break;
					// if no waiting threads, about 113ns
					// else lead to thread switching
					std::this_thread::yield();
					if (!m_lock.load(std::memory_order_relaxed)) break;
				}
			} while (m_lock.exchange(true, std::memory_order_acquire)); // Exchange is more efficient than CAS
		}

		bool try_lock() {
			return !m_lock.exchange(true, std::memory_order_acquire);
		}

		void unlock()
		{
			m_lock.store(false, std::memory_order_release);
		}

	private:
		std::atomic_bool m_lock;
	};

}
