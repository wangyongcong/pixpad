#pragma once

#include "render_target.h"
#include "surface.h"
#include "util.h"

namespace wyc
{
	class CSpwRenderTarget : public CRenderTarget
	{
	public:
		CSpwRenderTarget();
		virtual ~CSpwRenderTarget() override;
		virtual bool create(unsigned width, unsigned height, unsigned format) override;

		inline CSurface& get_color_buffer()
		{
			return m_color_buffer;
		}

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwRenderTarget)

		CSurface m_color_buffer;
		EPixelFormat m_color_fmt;
	};

} // namespace wyc
