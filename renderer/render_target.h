#pragma once

#include <memory>

namespace wyc
{
	enum render_target_format
	{
		COLOR_MASK = 0xFF,
		COLOR_SHIFT = 0,
		COLOR_R8G8B8A8 = 1,

		DEPTH_MASK = 0x300,
		DEPTH_SHIFT = 8,
		DEPTH_16 = 0x100,
		DEPTH_32 = 0x200,
		
		STENCIL_MASK = 0xC00,
		STENCIL_SHIFT = 10,
		STENCIL_8 = 0x400,
		STENCIL_16 = 0x800,
	};

	inline unsigned get_color_format(unsigned format)
	{
		return (format & COLOR_MASK);
	}

	inline unsigned get_depth_format(unsigned format)
	{
		return (format & DEPTH_MASK);
	}

	inline unsigned get_stencil_format(unsigned format)
	{
		return (format & STENCIL_MASK);
	}

	class render_target
	{
	public:
		virtual ~render_target() {}
		virtual bool create(unsigned width, unsigned height, unsigned format) = 0;
	};

} // namespace wyc