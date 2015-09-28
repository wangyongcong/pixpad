#include "spw_render_target.h"

namespace wyc
{
	CSpwRenderTarget::CSpwRenderTarget()
	{
	}

	CSpwRenderTarget::~CSpwRenderTarget()
	{
	}

	bool CSpwRenderTarget::create(unsigned width, unsigned height, unsigned format)
	{
		m_color_buffer.release();
		unsigned frag_size = 0, alignment = 0;
		EPixelFormat color_fmt = get_color_format(format);
		switch (color_fmt)
		{
		case SPR_COLOR_R8G8B8A8:
		case SPR_COLOR_B8G8R8A8:
			frag_size = 4;
			alignment = 4;
			break;
		default:
			return false;
		}
		if (!m_color_buffer.storage(width, height, frag_size, alignment))
			return false;
		m_color_fmt = color_fmt;
		return true;
	}

} // namespace wyc