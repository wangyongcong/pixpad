#include "stdafx.h"
#include "resource.h"
#include <locale>
#include "log.h"
#include "win_app.h"
#include "util.h"
#include "glrender.h"


// Global logger
wyc::xlogger *g_log = nullptr;
// Timer ID for log flushing
#define ID_TIMER_LOG 1
// Flush logger on time
void CALLBACK TimerFlushLog(HWND hwnd, UINT umsg, UINT_PTR id, DWORD time)
{
	if (id == ID_TIMER_LOG)
		g_log->flush();
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace wyc
{

	// Application singleton
	xwin_app *xwin_app::instance = nullptr;

	bool xwin_app::initialize(const std::wstring &app_name, HINSTANCE hInstance, size_t win_width , size_t win_height, LPTSTR cmd_line)
	{
		/*
		std::string app_dir;
		wchar_t path[MAX_PATH];
		unsigned len = ::GetModuleFileName(NULL, path, MAX_PATH);
		if (len>0) {
			wchar_t *pend = wcsrchr(path, L'\\');
			if (pend) 
				pend[1] = 0;
			::SetCurrentDirectory(path);
			wstr2str(app_dir, path);
		}*/
		if (!g_log)
		{
			std::string mbs_name;
			wstr2str(mbs_name, app_name);
			g_log = new wyc::xlogger();
			g_log->create(mbs_name.c_str());
			debug("starting...");
		}
		if (!xwin_app::instance)
		{
			return false;
		}
		std::wstring wnd_cls = app_name;
		wnd_cls += L"MainWindow";
		// register main window class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
		wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = wnd_cls.c_str();
		RegisterClassEx(&wcex);
		if (win_width == 0 || win_height == 0)
			win_width = CW_USEDEFAULT;
		HWND hMainWnd = CreateWindow(wnd_cls.c_str(), app_name.c_str(), (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME,
			CW_USEDEFAULT, 0, win_width, win_height, NULL, NULL, hInstance, NULL);
		if (!hMainWnd)
		{
			return false;
		}
		xwin_app::instance->m_hinst = hInstance;
		xwin_app::instance->m_hwnd_main = hMainWnd;

		// Set timer for log flushing 
		SetTimer(hMainWnd, ID_TIMER_LOG, 500, &TimerFlushLog);

		// create OpenGL window
		RECT rectClient;
		GetClientRect(hMainWnd, &rectClient);
		int client_w = rectClient.right - rectClient.left;
		int client_h = rectClient.bottom - rectClient.top;
		// Create a temporary window to initialize driver
		HWND hTmpWnd = wyc::gl_create_window(hInstance, hMainWnd, 0, 0, client_w, client_h);
		int pixel_format = wyc::gl_detect_drivers(hTmpWnd);
		DestroyWindow(hTmpWnd);
		if (!pixel_format)
		{
			return false;
		}
		// Create the real target window
		HWND hTargetWnd = wyc::gl_create_window(hInstance, hMainWnd, 0, 0, client_w, client_h);
		if (!wyc::gl_create_context(hTargetWnd, pixel_format))
		{
			return false;
		}
		xwin_app::instance->m_view_w = client_w;
		xwin_app::instance->m_view_h = client_h;
		// Do not response user input
		EnableWindow(hTargetWnd, FALSE);
		ShowWindow(hTargetWnd, SW_NORMAL);

		// Show OpenGL infomation
		const char *version = (const char*)glGetString(GL_VERSION);
		const char *glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		const char *vendor = (const char*)glGetString(GL_VENDOR);
		const char *device = (const char*)glGetString(GL_RENDERER);
		info("%s %s", vendor, device);
		info("OpenGL %s (GLSL %s)", version, glsl_version);
		info("GLEW version %s", glewGetString(GLEW_VERSION));

		xwin_app::instance->on_start();

		ShowWindow(hMainWnd, SW_NORMAL);
		UpdateWindow(hMainWnd);

		return true;
	}

	void xwin_app::destroy()
	{
		if (xwin_app::instance)
		{
			delete xwin_app::instance;
			xwin_app::instance = nullptr;
		}
		if (g_log)
		{
			g_log->flush();
			delete g_log;
			g_log = nullptr;
		}
	}

	void xwin_app::run()
	{
		HACCEL hAccelTable = NULL;
		MSG msg;
		while (true)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) {
					on_close();
					break;
				}
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				this->on_render();
				Sleep(1);
			}
		}
	}

}; // end of namespace wyc
