#pragma once
#include <future>
#include <queue>
#include <functional>
#include "absl/synchronization/mutex.h"
#include "absl/base/thread_annotations.h"
#include "common/common_macros.h"

namespace wyc
{
	class TaskThread
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(TaskThread)
	public:
		TaskThread();
		~TaskThread();
		void start();
		void stop();

		template<class Function, class... Args>
		std::future<std::invoke_result_t<Function, Args...>> enqueue(Function &&callable, Args &&...args)
		{
			using return_t = std::invoke_result_t<Function, Args...>;

			auto task = std::make_shared<std::packaged_task<return_t()>>(
				std::bind(std::forward<Function>(callable), std::forward<Args>(args)...));

			{
				absl::MutexLock lock(&m_mutex);
				m_task_queue.emplace([task]()
				{
				   (*task)();
				});
			}

			return task->get_future();
		}

	private:
		bool has_tasks() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(m_mutex)
		{
			return m_stopped || !m_task_queue.empty();
		}
		// protects m_task_queue
		absl::Mutex m_mutex; 
		std::thread m_worker;
		std::queue<std::function<void()>> m_task_queue ABSL_GUARDED_BY(m_mutex);
		bool m_stopped ABSL_GUARDED_BY(m_mutex);
	};
}
