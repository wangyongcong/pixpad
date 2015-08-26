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
		int m_pixel_fmt;
	};

} // namespace wyc