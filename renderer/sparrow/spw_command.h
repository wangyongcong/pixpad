#pragma once

#include "OpenEXR/ImathColorAlgo.h"
#include "render_command.h"
#include "spw_renderer.h"

namespace wyc
{
	template<>
	inline bool spw_handler<cmd_present>(renderer * renderer, render_command *)
	{
		renderer->present();
		return true;
	}

	template<>
	inline bool spw_handler<cmd_clear>(renderer * _renderer, render_command * _cmd)
	{
		spw_renderer *renderer = dynamic_cast<spw_renderer*>(_renderer);
		assert(renderer);
		cmd_clear *cmd = static_cast<cmd_clear*>(_cmd);
		assert(cmd);
		if (!renderer->m_rt)
		{
			return false;
		}
		xsurface& surf = renderer->m_rt->get_color_buffer();
		uint32_t v = Imath::rgb2packed(cmd->color);
		surf.clear(v);
		return true;
	}


} // namespace wyc