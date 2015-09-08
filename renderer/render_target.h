#pragma once

#include <memory>

namespace wyc
{
	enum spr_pixel_format
	{
		// color buffer
		SPR_COLOR_MASK = 0xFF,
		SPR_COLOR_SHIFT = 0,
		SPR_COLOR_R8G8B8A8 = 1,
		SPR_COLOR_B8G8R8A8 = 2,

		// depth buffer
		SPR_DEPTH_MASK = 0x300,
		SPR_DEPTH_SHIFT = 8,
		SPR_DEPTH_16 = 0x100,
		SPR_DEPTH_32 = 0x200,
		
		// stencil buffer
		SPR_STENCIL_MASK = 0xC00,
		SPR_STENCIL_SHIFT = 10,
		SPR_STENCIL_8 = 0x400,
		SPR_STENCIL_16 = 0x800,
	};

	inline spr_pixel_format get_color_format(unsigned format)
	{
		return spr_pixel_format(format & SPR_COLOR_MASK);
	}

	inline spr_pixel_format get_depth_format(unsigned format)
	{
		return spr_pixel_format(format & SPR_DEPTH_MASK);
	}

	inline spr_pixel_format get_stencil_format(unsigned format)
	{
		return spr_pixel_format(format & SPR_STENCIL_MASK);
	}

	class render_target
	{
	public:
		virtual ~render_target() {}
		virtual bool create(unsigned width, unsigned height, unsigned format) = 0;
	};

} // namespace wyc