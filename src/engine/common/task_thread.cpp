#include "task_thread.h"

namespace wyc
{
	TaskThread::TaskThread()
	{
	}

	TaskThread::~TaskThread()
	{
		stop();
		m_worker.join();
	}

	void TaskThread::start()
	{
		m_stopped = false;
		m_worker = std::move(std::thread([this]()
		{
			while(true)
			{
				std::function<void()> task;
				{
					std::unique_lock lock(m_mutex);
					m_cv.wait(lock, [this]()
					{
						return is_stopped() || !m_task_queue.empty();
					});
					if(is_stopped())
					{
						break;
					}
					task = std::move(m_task_queue.front());
					m_task_queue.pop();
				}
				task();
			}
		}));
	}

	void TaskThread::stop()
	{
		m_stopped.store(true, std::memory_order_release);
		m_cv.notify_all();
	}

}

