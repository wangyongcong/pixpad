#pragma once

#include <future>
#include <OpenEXR/ImathColor.h>
#include "render_target.h"
#include "render_command.h"
#include "ring_queue.h"

namespace wyc
{
	class CRenderer
	{
	public:
		CRenderer();
		virtual ~CRenderer();
		virtual void set_render_target(std::shared_ptr<CRenderTarget> rt) = 0;
		virtual std::shared_ptr<CRenderTarget> get_render_target() = 0;
		virtual void process() = 0;
		void wait_for_ready();
		void set_ready();
		void present();
		void end_frame();
		// Create a render command.
		template<class Command, class ...Args>
		Command* new_command(Args&& ...args);
		// Enqueue command
		bool enqueue(RenderCommand *cmd);

	protected:
		CCommandAllocator m_cmd_alloc;
		CRingQueue<RenderCommand*> m_cmd_queue;
		std::promise<void> m_is_ready;
		std::future<void> m_is_done;
	};

	inline void CRenderer::end_frame()
	{
		m_is_done.wait();
		m_cmd_alloc.reset();
	}

	template<class Command, class ...Args>
	inline Command * CRenderer::new_command(Args&& ...args)
	{
		void *ptr = m_cmd_alloc.alloc(sizeof(Command));
		if (!ptr)
			return nullptr;
		Command *cmd = new(ptr) Command(std::forward<Args>(args)...);
		cmd->set_tid(Command::tid);
		return cmd;
	}

	inline bool CRenderer::enqueue(RenderCommand *cmd)
	{
		return m_cmd_queue.enqueue(cmd);
	}

} // namespace wyc