#include "stdafx.h"
#include "resource.h"
#include <locale>
#include "log.h"
#include "windows_application.h"
#include "util.h"

// Global logger
wyc::CLogger *g_log = nullptr;
// Timer ID for log flushing
#define ID_TIMER_LOG 1
// Flush logger on time
void CALLBACK TimerFlushLog(HWND hwnd, UINT umsg, UINT_PTR id, DWORD time)
{
	if (id == ID_TIMER_LOG)
		g_log->flush();
}

LRESULT CALLBACK WindowProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: custom GUI paint 
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

namespace wyc
{

	bool CWindowsApplication::initialize(const std::wstring &app_name, CGame* game_inst, HINSTANCE hInstance, size_t win_width , size_t win_height, LPTSTR cmd_line)
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
			g_log = new wyc::CDebugLogger();
			debug("starting...");
		}

		std::wstring wnd_cls = app_name;
		wnd_cls += L"MainWindow";
		// register main window class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProcess;
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
		DWORD style = (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME;
		RECT client_rect;
		if (win_width == 0 || win_height == 0)
		{
			win_width = CW_USEDEFAULT;
			win_height = CW_USEDEFAULT;
		}
		else
		{
			client_rect = {
				0, 0, int(win_width), int(win_height)
			};
			AdjustWindowRect(&client_rect, style, FALSE);
			win_width = client_rect.right - client_rect.left;
			win_height = client_rect.bottom - client_rect.top;
		}
		HWND hMainWnd = CreateWindow(wnd_cls.c_str(), app_name.c_str(), style,
			CW_USEDEFAULT, CW_USEDEFAULT, win_width, win_height, NULL, NULL, hInstance, NULL);
		if (!hMainWnd)
		{
			return false;
		}

		this->m_hinst = hInstance;
		this->m_hwnd_main = hMainWnd;

		//// set timer for log flushing 
		//SetTimer(hMainWnd, ID_TIMER_LOG, 500, &TimerFlushLog);

		// create OpenGL window
		GetClientRect(hMainWnd, &client_rect);
		int client_w = client_rect.right - client_rect.left;
		int client_h = client_rect.bottom - client_rect.top;
		this->m_view_w = client_w;
		this->m_view_h = client_h;

		this->m_game = game_inst;
		m_game->on_start();

		ShowWindow(hMainWnd, SW_NORMAL);
		UpdateWindow(hMainWnd);

		return true;
	}

	void CWindowsApplication::start()
	{
		HACCEL hAccelTable = NULL;
		MSG msg;
		while (true)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) {
					m_game->on_close();
					break;
				}
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				on_event(&msg);
			}
			m_game->on_update();
		}
		close();
	}

	void CWindowsApplication::close()
	{
		OutputDebugString(L"windows application is closing...\n");
		if (g_log)
		{
			g_log->flush();
			delete g_log;
			g_log = nullptr;
		}
	}

	void CWindowsApplication::resize(unsigned view_w, unsigned view_h)
	{
		if (!m_hwnd_main)
			return;
		RECT cur_client_rect;
		GetClientRect(m_hwnd_main, &cur_client_rect);
		cur_client_rect.right = cur_client_rect.left + view_w;
		cur_client_rect.bottom = cur_client_rect.top + view_h;
		long style = GetWindowLong(m_hwnd_main, GWL_STYLE);
		long exstyle = GetWindowLong(m_hwnd_main, GWL_EXSTYLE);
		if (!AdjustWindowRectEx(&cur_client_rect, style, FALSE, exstyle))
		{
			error("Can't adjust window client rect!");
		}
		if (SetWindowPos(m_hwnd_main, HWND_TOP, 0, 0, cur_client_rect.right - cur_client_rect.left,
			cur_client_rect.bottom - cur_client_rect.top, SWP_NOMOVE))
		{
			m_view_w = view_w;
			m_view_h = view_h;
			debug("resize window: (%d, %d)", view_w, view_h);
		}
		else
		{
			error("Can't set window size!");
		}
	}

	void CWindowsApplication::on_event(void * ev)
	{
		MSG *msg = (MSG*)ev;
		switch (msg->message)
		{
		case WM_SIZE:
			m_game->on_resize(LOWORD(msg->wParam), HIWORD(msg->lParam));
			break;
		case WM_LBUTTONDOWN:
			SetCapture(os_window());
			m_game->on_mouse_button_down(MOUSE_BUTTON_LEFT, LOWORD(msg->lParam), HIWORD(msg->lParam));
			break;
		case WM_LBUTTONUP:
			m_game->on_mouse_button_up(MOUSE_BUTTON_LEFT, LOWORD(msg->lParam), HIWORD(msg->lParam));
			ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			m_game->on_mouse_move(LOWORD(msg->lParam), HIWORD(msg->lParam));
			break;
		case WM_MOUSEWHEEL:
			break;
		case WM_KEYDOWN:
			m_game->on_key_down(msg->wParam);
			break;
		}
	}


}; // end of namespace wyc
