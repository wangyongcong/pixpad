#include "stdafx.h"
#include "view_sparrow.h"

#include "log.h"
#include "glutil.h"
#include "windows_application.h"

namespace wyc
{
	bool view_sparrow::create(HWND parent, int x, int y, unsigned w, unsigned h)
	{
		windows_application* app_inst = dynamic_cast<windows_application*>(application::get_instance());
		if (!app_inst)
		{
			return false;
		}

		// create a temporary window to initialize driver
		HINSTANCE os_instance = app_inst->os_instance();
		HWND main_wnd = app_inst->os_window();
		HWND tmp_wnd = wyc::gl_create_window(os_instance, main_wnd, 0, 0, w, h);
		int pixel_format = wyc::gl_detect_drivers(tmp_wnd);
		DestroyWindow(tmp_wnd);
		if (!pixel_format)
		{
			return false;
		}

		// create the real target window
		HWND target_wnd = wyc::gl_create_window(os_instance, main_wnd, 0, 0, w, h);
		if (!wyc::gl_create_context(target_wnd, pixel_format))
		{
			return false;
		}
		// do not response user input
		EnableWindow(target_wnd, FALSE);
		ShowWindow(target_wnd, SW_NORMAL);

		// set member
		m_hwnd = target_wnd;

		return true;
	}

	void view_sparrow::on_render()
	{
	}
} // namespace wyc