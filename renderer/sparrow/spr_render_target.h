#pragma once

#include "render_target.h"
#include "surface.h"
#include "util.h"

namespace wyc
{
	class spr_render_target : public render_target
	{
	public:
		spr_render_target();
		virtual ~spr_render_target() override;
		virtual bool create(unsigned width, unsigned height, unsigned format) override;

		inline xsurface& get_color_buffer()
		{
			return m_color_buffer;
		}

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spr_render_target)

		xsurface m_color_buffer;
		spr_pixel_format m_color_fmt;
	};

} // namespace wyc
