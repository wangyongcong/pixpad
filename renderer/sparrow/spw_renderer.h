#pragma once

#include "renderer.h"
#include "spw_render_target.h"
#include "ring_queue.h"
#include "render_command.h"
#include "util.h"

namespace wyc
{
	template<class Command>
	inline bool spw_handler(renderer * renderer, render_command * _cmd)
	{
		static_assert(0, "Not implemented.");
	}

	class spw_renderer : public renderer
	{
	public:
		spw_renderer();
		virtual ~spw_renderer() override;
		
		virtual void set_render_target(std::shared_ptr<render_target> rt) override;
		virtual std::shared_ptr<render_target> get_render_target() override;
		virtual void process() override;
		virtual void present() override;
		virtual bool enqueue(render_command *cmd) override;

		// Create a render command.
		template<class Command, class ...Args>
		Command* new_command(Args&& ...args);
	
		// Internal implementation of render result presentation.
		std::function<void(void)> spw_present;
	
	protected:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spw_renderer)

		template<class Command>
		friend bool spw_handler(renderer*, render_command*);

		std::shared_ptr<spw_render_target> m_rt;
		ring_queue<render_command*> m_cmd_queue;
		std::vector<render_command*> m_cmd_buffer;
		command_allocator m_cmd_alloc;
	};

	template<class Command, class ...Args>
	inline Command * spw_renderer::new_command(Args&& ...args)
	{
		void *ptr = m_cmd_alloc.alloc(sizeof(Command));
		if (!ptr)
			return nullptr;
		Command *cmd = new(ptr) Command(std::forward<Args>(args)...);
		cmd->set_tid(Command::tid);
		return cmd;
	}

	inline bool spw_renderer::enqueue(render_command *cmd)
	{
		return m_cmd_queue.enqueue(cmd);
	}

} // namespace wyc
