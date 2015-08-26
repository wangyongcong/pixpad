#include "stdafx.h"
#include "view_sparrow.h"

#include <thread>

#include "log.h"
#include "glutil.h"
#include "windows_application.h"

namespace wyc
{
	bool view_sparrow::initialize(int x, int y, unsigned w, unsigned h)
	{
		windows_application* app_inst = dynamic_cast<windows_application*>(application::get_instance());
		if (!app_inst)
		{
			return false;
		}

		HINSTANCE os_instance = app_inst->os_instance();
		HWND main_wnd = app_inst->os_window();
		int pixel_format = wyc::gl_detect_drivers(NULL);
		if (!pixel_format)
		{
			// driver is not ready, create a temporary window to initialize driver
			HWND tmp_wnd = wyc::gl_create_window(os_instance, main_wnd, 0, 0, w, h);
			pixel_format = wyc::gl_detect_drivers(tmp_wnd);
			DestroyWindow(tmp_wnd);
			if (!pixel_format)
			{
				return false;
			}
		}

		// create the real target window
		HWND target_wnd = wyc::gl_create_window(os_instance, main_wnd, 0, 0, w, h);
		// do not response user input
		EnableWindow(target_wnd, FALSE);
		ShowWindow(target_wnd, SW_NORMAL);

		// set member
		m_hwnd = target_wnd;
		m_pixel_fmt = pixel_format;

		return true;
	}

	void view_sparrow::on_render()
	{
		auto thread_id = std::this_thread::get_id();
		debug("start render on thread[%d], sparrow view", thread_id);

		if (!wyc::gl_create_context(m_hwnd, m_pixel_fmt))
		{
			debug("failed to create renderer");
			return;
		}

		while (!application::get_instance()->is_exit())
		{
			std::this_thread::sleep_for(std::chrono::microseconds(30));
		}

		debug("exit thread[%d]", thread_id);
	}
} // namespace wyc