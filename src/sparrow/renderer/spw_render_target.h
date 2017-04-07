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
		inline CSurface& get_depth_buffer()
		{
			return m_depth_buffer;
		}
		inline CSurface& get_stencil_buffer()
		{
			return m_stencil_buffer;
		}
		inline bool has_depth() const 
		{
			return !m_depth_buffer.empty();
		}
		inline bool has_stencil() const
		{
			return !m_stencil_buffer.empty();
		}

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwRenderTarget)
		CSurface m_color_buffer;
		CSurface m_depth_buffer;
		CSurface m_stencil_buffer;
	};

} // namespace wyc
