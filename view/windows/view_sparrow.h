#pragma once
#include "view_base.h"

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
		ID2D1Factory *m_d2d_factory;
		ID2D1HwndRenderTarget *m_d2d_rt;
	};

} // namespace wyc