#pragma once

#include <memory>

namespace wyc
{
	enum EPixelFormat
	{
		SPW_INVALID_FORMAT = 0,

		// color buffer
		SPW_COLOR_MASK = 0xFF,
		SPW_COLOR_SHIFT = 0,
		SPW_COLOR_R8G8B8A8 = 1,
		SPW_COLOR_RGBA_F32 = 2,

		// depth buffer
		SPW_DEPTH_MASK = 0x300,
		SPW_DEPTH_SHIFT = 8,
		SPW_DEPTH_16 = 0x100,
		SPW_DEPTH_32 = 0x200,
		
		// stencil buffer
		SPW_STENCIL_MASK = 0xC00,
		SPW_STENCIL_SHIFT = 10,
		SPW_STENCIL_8 = 0x400,
		SPW_STENCIL_16 = 0x800,
	};

	inline EPixelFormat get_color_format(unsigned format)
	{
		return EPixelFormat(format & SPW_COLOR_MASK);
	}

	inline EPixelFormat get_depth_format(unsigned format)
	{
		return EPixelFormat(format & SPW_DEPTH_MASK);
	}

	inline EPixelFormat get_stencil_format(unsigned format)
	{
		return EPixelFormat(format & SPW_STENCIL_MASK);
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