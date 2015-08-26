#pragma once

#include <string>
#include "view_base.h"

namespace wyc
{
	class view_ogl3 : public view_base
	{
	public:
		view_ogl3();
		~view_ogl3() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h);
		virtual void set_text(const wchar_t *text);
		virtual void on_render();
	protected:
		HWND m_hwnd;
		int m_pixel_fmt;
	};

} // namespace wyc