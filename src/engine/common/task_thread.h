#pragma once
#include <future>
#include <queue>
#include <functional>
#include <condition_variable>
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
		bool is_stopped() const
		{
			return m_stopped.load(std::memory_order_acquire);
		}

		template<class Function, class... Args>
		std::future<std::invoke_result_t<Function(Args...)>> enqueue(Function &&callable, Args &&...args)
		{
			using return_t = std::invoke_result_t<Function(Args...)>;

			auto task = std::make_shared<std::packaged_task<return_t()>>(
				std::bind(std::forward<Function>(callable), std::forward<Args>(args))
				);

			std::unique_lock lock(m_mutex);
			m_task_queue.emplace([task]()
			{
			   (*task)();
			});
			m_cv.notify_one();

			return task->get_future();
		}

	private:
		std::mutex m_mutex;
		std::condition_variable m_cv;
		std::atomic<bool> m_stopped;
		std::thread m_worker;
		std::queue<std::function<void()>> m_task_queue;
	};
}
