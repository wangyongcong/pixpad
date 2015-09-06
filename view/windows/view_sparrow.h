#pragma once
#include "view_base.h"

namespace wyc
{
	class view_sparrow : public view_base
	{
	public:
		virtual ~view_sparrow() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h);
		virtual void on_render();
		virtual void set_text(const wchar_t *text) {}
	protected:
		HWND m_hwnd;
		ID2D1Factory *m_d2d_factory;
	};

} // namespace wyc