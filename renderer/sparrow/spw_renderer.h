#pragma once

#include "renderer.h"
#include "spw_render_target.h"
#include "ring_queue.h"
#include "render_command.h"
#include "util.h"

namespace wyc
{
	class spw_renderer : public renderer
	{
	public:
		spw_renderer();
		virtual ~spw_renderer() override;
		virtual void set_render_target(std::shared_ptr<render_target> rt) override;
		virtual std::shared_ptr<render_target> get_render_target() override;
		virtual void clear(const Imath::C3f &c) override;
		
		// producer thread
		template<class Command, class ...Args>
		Command* new_command(Args&& ...args);
		bool enqueue(render_command *cmd);

		// render thread
	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spw_renderer)

		std::shared_ptr<spw_render_target> m_rt;
		ring_queue<render_command*> m_cmd_queue;
		command_allocator m_cmd_alloc;
	};

	template<class Command>
	inline bool spw_handler(renderer * renderer, render_command * _cmd)
	{
		Command *cmd = static_cast<Command*>(_cmd);
		assert(cmd);
		return false;
	}

	template<>
	inline bool spw_handler<cmd_clear>(renderer * renderer, render_command * _cmd)
	{
		cmd_clear *cmd = static_cast<cmd_clear*>(_cmd);
		assert(cmd);
		return true;
	}

	template<class Command, class ...Args>
	inline Command * spw_renderer::new_command(Args&& ...args)
	{
		void *ptr = m_cmd_alloc.alloc(sizeof(Command));
		if (!ptr)
			return nullptr;
		Command *cmd = new(ptr) Command(std::forward<Args>(args)...);
		cmd->handler = &spw_handler<Command>;
		return cmd;
	}

	inline bool spw_renderer::enqueue(render_command *cmd)
	{
		return m_cmd_queue.enqueue(cmd);
	}

} // namespace wyc
