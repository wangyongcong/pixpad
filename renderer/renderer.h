#pragma once

#include "OpenEXR/ImathColor.h"
#include "render_target.h"
#include "render_command.h"
#include "ring_queue.h"

namespace wyc
{
	class renderer
	{
	public:
		renderer();
		virtual ~renderer();
		virtual void set_render_target(std::shared_ptr<render_target> rt) = 0;
		virtual std::shared_ptr<render_target> get_render_target() = 0;
		virtual void process() = 0;
		virtual void present() = 0;
		// Create a render command.
		template<class Command, class ...Args>
		Command* new_command(Args&& ...args);
		// Enqueue command
		bool enqueue(render_command *cmd);

	protected:
		command_allocator m_cmd_alloc;
		ring_queue<render_command*> m_cmd_queue;
	};

	template<class Command, class ...Args>
	inline Command * renderer::new_command(Args&& ...args)
	{
		void *ptr = m_cmd_alloc.alloc(sizeof(Command));
		if (!ptr)
			return nullptr;
		Command *cmd = new(ptr) Command(std::forward<Args>(args)...);
		cmd->set_tid(Command::tid);
		return cmd;
	}

	inline bool renderer::enqueue(render_command *cmd)
	{
		return m_cmd_queue.enqueue(cmd);
	}

} // namespace wyc