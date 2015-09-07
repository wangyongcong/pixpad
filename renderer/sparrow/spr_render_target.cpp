#include "spr_render_target.h"

namespace wyc
{
	spr_render_target::spr_render_target()
	{
	}

	spr_render_target::~spr_render_target()
	{
	}

	bool spr_render_target::create(unsigned width, unsigned height, unsigned format)
	{
		m_color_buffer.release();
		unsigned frag_size = 0, alignment = 0;
		unsigned fmt_color = get_color_format(format);
		switch (fmt_color)
		{
		case COLOR_R8G8B8A8:
			frag_size = 32;
			alignment = 4;
			break;
		default:
			return false;
		}
		if (!m_color_buffer.storage(width, height, frag_size, alignment))
			return false;
		return true;
	}

} // namespace wyc