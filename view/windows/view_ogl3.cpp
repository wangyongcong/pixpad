#include "stdafx.h"
#include "view_ogl3.h"

#include <thread>

#include "log.h"
#include "ogl/glutil.h"
#include "windows_application.h"


namespace wyc
{
	CViewOgl3::CViewOgl3()
	{
		m_hwnd = NULL;
	}

	bool CViewOgl3::initialize(int x, int y, unsigned w, unsigned h)
	{
		CWindowsApplication* app_inst = dynamic_cast<CWindowsApplication*>(CApplication::get_instance());
		if (!app_inst)
		{
			return false;
		}

		HINSTANCE os_instance = app_inst->os_instance();
		HWND main_wnd = app_inst->os_window();

		// check OpenGL driver
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
		HWND target_wnd = wyc::gl_create_window(os_instance, main_wnd, x, y, w, h);
		// do not response user input
		EnableWindow(target_wnd, FALSE);
		ShowWindow(target_wnd, SW_NORMAL);
		
		// set member
		m_hwnd = target_wnd;
		m_pixel_fmt = pixel_format;

		m_view_pos.setValue(x, y);
		m_view_size.setValue(int(w), int(h));

		debug("OpenGL view at (%d, %d, %d, %d)", x, y, x + w, y + h);

		return true;
	}

	void CViewOgl3::set_text(const wchar_t *text)
	{
	}

	void CViewOgl3::on_render()
	{
		auto thread_id = std::this_thread::get_id();
		debug("start render on thread[0x%x], ogl3 view", thread_id);

		if (!wyc::gl_create_context(m_hwnd, m_pixel_fmt))
		{
			debug("failed to create renderer");
			return;
		}

		// show OpenGL infomation
		const char *version = (const char*)glGetString(GL_VERSION);
		const char *glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		const char *vendor = (const char*)glGetString(GL_VENDOR);
		const char *device = (const char*)glGetString(GL_RENDERER);
		info("%s %s", vendor, device);
		info("OpenGL %s (GLSL %s)", version, glsl_version);
		info("GLEW version %s", glewGetString(GLEW_VERSION));

		while (!CApplication::get_instance()->is_exit())
		{
			glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gl_get_context()->swap_buffers();
			std::this_thread::sleep_for(std::chrono::microseconds(30));
		}

		debug("exit thread[0x%x]", thread_id);
	}

	void CViewOgl3::get_position(int & x, int & y)
	{
		x = m_view_pos.x;
		y = m_view_pos.y;
	}

	void CViewOgl3::get_size(unsigned & width, unsigned & height)
	{
		width = unsigned(m_view_size.x);
		height = unsigned(m_view_size.y);
	}

} // namespace wyc
