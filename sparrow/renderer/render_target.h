#pragma once

#include <memory>

namespace wyc
{
	enum EPixelFormat
	{
		SPR_INVALID_FORMAT = 0,

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

	inline EPixelFormat get_color_format(unsigned format)
	{
		return EPixelFormat(format & SPR_COLOR_MASK);
	}

	inline EPixelFormat get_depth_format(unsigned format)
	{
		return EPixelFormat(format & SPR_DEPTH_MASK);
	}

	inline EPixelFormat get_stencil_format(unsigned format)
	{
		return EPixelFormat(format & SPR_STENCIL_MASK);
	}

	class CRenderTarget
	{
	public:
		virtual ~CRenderTarget() {}
		virtual bool create(unsigned width, unsigned height, unsigned format) = 0;
		
		inline void get_size(unsigned &width, unsigned &height) const
		{
			width = m_rt_width;
			height = m_rt_height;
		}
		inline unsigned width() const {
			return m_rt_width;
		}
		inline unsigned height() const {
			return m_rt_height;
		}

	protected:
		unsigned m_rt_width, m_rt_height;
	};

} // namespace wyc