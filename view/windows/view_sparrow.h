#pragma once
#include "view_base.h"
#include "sparrow/spr_renderer.h"

namespace wyc
{
	class view_sparrow : public view_base
	{
	public:
		view_sparrow();
		virtual ~view_sparrow();
		virtual bool initialize(int x, int y, unsigned w, unsigned h);
		virtual void on_render();
		virtual void set_text(const wchar_t *text) {}
	protected:
		HWND m_hwnd;
		ID2D1Factory *m_d2d_factory;
		ID2D1HwndRenderTarget *m_d2d_rt;
		ID2D1Bitmap *m_bitmap;
		std::shared_ptr<spr_renderer> m_renderer;
		std::shared_ptr<spr_render_target> m_target;
	};

} // namespace wyc