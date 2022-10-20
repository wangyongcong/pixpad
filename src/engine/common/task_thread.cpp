#include "common/task_thread.h"
#include "stb/stb_log.h"

namespace wyc
{
	TaskThread::TaskThread()
	{
		m_stopped = false;
	}

	TaskThread::~TaskThread()
	{
		if(m_worker.joinable())
		{
			stop();
		}
	}

	void TaskThread::start()
	{
		m_worker = std::move(std::thread([this]()
		{
			while(true)
			{
				std::function<void()> task;
				{
					absl::MutexLock lock(&m_mutex);
					m_mutex.Await(absl::Condition(this, &TaskThread::has_tasks));
					if(m_stopped)
					{
						break;
					}
					task = std::move(m_task_queue.front());
					m_task_queue.pop();
				}
				task();
			}
			log_info("Task thread exit");
		}));
	}

	void TaskThread::stop()
	{
		{
			absl::MutexLock lock(&m_mutex);
			m_stopped = true;
		}
		m_worker.join();
	}

}

