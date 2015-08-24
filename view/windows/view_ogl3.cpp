#include "stdafx.h"
#include "view_ogl3.h"

#include "log.h"
#include "glutil.h"
#include "windows_application.h"


namespace wyc
{
	view_ogl3::view_ogl3()
	{
		m_hwnd = NULL;
	}

	bool view_ogl3::create(HWND parent, int x, int y, unsigned w, unsigned h)
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

		// show OpenGL infomation
		const char *version = (const char*)glGetString(GL_VERSION);
		const char *glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		const char *vendor = (const char*)glGetString(GL_VENDOR);
		const char *device = (const char*)glGetString(GL_RENDERER);
		info("%s %s", vendor, device);
		info("OpenGL %s (GLSL %s)", version, glsl_version);
		info("GLEW version %s", glewGetString(GLEW_VERSION));
		info("GL View %d x %d", w, h);

		return true;
	}

	void view_ogl3::set_text(const wchar_t *text)
	{
	}

	void view_ogl3::on_render()
	{
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gl_get_context()->swap_buffers();
	}

} // namespace wyc
