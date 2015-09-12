#pragma once

#include "renderer.h"
#include "spr_render_target.h"
#include "render_queue.h"
#include "util.h"

namespace wyc
{
	class spr_renderer : public renderer
	{
	public:
		spr_renderer();
		virtual ~spr_renderer() override;
		virtual void set_render_target(std::shared_ptr<render_target> rt) override;
		virtual std::shared_ptr<render_target> get_render_target() override;
		virtual void clear(const Imath::C3f &c) override;
		
		template<class Command>
		Command* new_command(command_id id);

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spr_renderer)

		std::shared_ptr<spr_render_target> m_rt;
		render_queue<spr_renderer> m_cmd_queue;
	};

	template<class Command>
	inline bool spr_command_handler(renderer * renderer, render_command * _cmd)
	{
		Command *cmd = static_cast<Command*>(_cmd);
		assert(cmd);
		return false;
	}

	template<>
	inline bool spr_command_handler<cmd_clear>(renderer * renderer, render_command * _cmd)
	{
		cmd_clear *cmd = static_cast<cmd_clear*>(_cmd);
		assert(cmd);
		return true;
	}

	template<class Command>
	struct spr_command : public Command
	{
		spr_command(command_id id) 
		{
			this->id = id;
			this->handler = &spr_command_handler<Command>;
		}
	};

	template<class Command>
	inline Command * spr_renderer::new_command(command_id id)
	{
		Command* cmd = new spr_command<Command>(id);
		return cmd;
	}

} // namespace wyc
