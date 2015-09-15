#pragma once

#include "renderer.h"
#include "spw_render_target.h"
#include "render_queue.h"
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
		
		template<class Command>
		Command* new_command();

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spw_renderer)

		std::shared_ptr<spw_render_target> m_rt;
		render_queue<spw_renderer> m_cmd_queue;
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

	template<class Command>
	inline Command * spw_renderer::new_command()
	{
		Command* cmd = new Command;
		cmd->id = id;
		cmd->handler = &spr_process<Command>;
		return cmd;
	}

} // namespace wyc
