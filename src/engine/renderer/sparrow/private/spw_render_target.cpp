#include "renderer/sparrow/spw_render_target.h"

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
		unsigned frag_size = 0, alignment = 4;
		EPixelFormat color_fmt = get_color_format(format);
		switch (color_fmt)
		{
		case SPW_COLOR_R8G8B8A8:
			frag_size = 4;
			break;
		case SPW_COLOR_RGBA_F32:
			frag_size = 16;
			break;
		default:
			return false;
		}
		if (!m_color_buffer.storage(width, height, frag_size, alignment))
			return false;
		EPixelFormat depth_format = get_depth_format(format);
		switch (depth_format)
		{
		case wyc::SPW_DEPTH_16:
		case wyc::SPW_DEPTH_32:
			frag_size = 4;
			break;
		default:
			frag_size = 0;
			break;
		}
		if (frag_size)
		{
			if (!m_depth_buffer.storage(width, height, frag_size, alignment))
			{
				m_color_buffer.release();
				return false;
			}
		}
		EPixelFormat stencil_format = get_stencil_format(format);
		switch (stencil_format)
		{
		case wyc::SPW_STENCIL_8:
			frag_size = 1;
			break;
		case wyc::SPW_STENCIL_16:
			frag_size = 2;
			break;
		default:
			frag_size = 0;
			break;
		}
		if (frag_size)
		{
			if (!m_stencil_buffer.storage(width, height, frag_size, alignment))
			{
				m_color_buffer.release();
				m_depth_buffer.release();
				return false;
			}
		}
		m_rt_width = width;
		m_rt_height = height;
		return true;
	}

} // namespace wyc