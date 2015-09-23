#pragma once

#include "OpenEXR/ImathColorAlgo.h"
#include "render_command.h"
#include "spw_renderer.h"

namespace wyc
{

#define SPW_CMD_HANDLER(cmd_type) template<> inline void spw_handler<cmd_type>(spw_renderer *renderer, render_command *_cmd)
	
#define get_cmd(cmd_type) static_cast<cmd_type*>(_cmd)

	SPW_CMD_HANDLER(cmd_test)
	{
		auto cmd = get_cmd(cmd_test);
		for (auto &v : *cmd->jobs)
		{
			v = 1;
		}
	}

	SPW_CMD_HANDLER(cmd_present)
	{
		renderer->spw_present();
		auto *cmd = get_cmd(cmd_present);
		cmd->is_done.set_value();
	}

	SPW_CMD_HANDLER(cmd_clear)
	{
		assert(renderer);
		auto *cmd = get_cmd(cmd_clear);
		assert(cmd);
		if (!renderer->m_rt)
		{
			return;
		}
		xsurface& surf = renderer->m_rt->get_color_buffer();
		uint32_t v = Imath::rgb2packed(cmd->color);
		surf.clear(v);
	}


} // namespace wyc