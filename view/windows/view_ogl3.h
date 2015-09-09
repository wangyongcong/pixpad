#pragma once

#include <string>
#include <OpenEXR/ImathVec.h>
#include "view_base.h"

namespace wyc
{
	class view_ogl3 : public view_base
	{
	public:
		view_ogl3();
		~view_ogl3() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h) override;
		virtual void set_text(const wchar_t *text) override;
		virtual void on_render() override;
		virtual void get_position(int &x, int &y) override;
		virtual void get_size(unsigned &width, unsigned &height) override;

	protected:
		HWND m_hwnd;
		int m_pixel_fmt;
		Imath::V2i m_view_pos;
		Imath::V2i m_view_size;
	};

} // namespace wyc