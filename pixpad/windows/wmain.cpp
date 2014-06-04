#include "stdafx.h"
#include "resource.h"
#include "pixpad.h"

#ifdef _DEBUG
	#pragma comment (lib, "mathexd.lib")
	#pragma comment (lib, "elogd.lib")
#else
	#pragma comment (lib, "mathex.lib")
	#pragma comment (lib, "elog.lib")
#endif 

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	using namespace wyc;
	setlocale(LC_ALL, "chs");
	std::wstring app_name = L"Pixpad";
	xwin_app::instance = new xapp_pixpad();
	if (!xwin_app::initialize(app_name, hInstance, lpCmdLine))
		return 1;
	xwin_app::instance->run();
	xwin_app::destroy();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using namespace wyc;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_COMMAND: // Window menu
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// switch menu command ID 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(xwin_app::instance->get_hinst(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			if (!xwin_app::instance->on_command(wmId, wmEvent))
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: custom GUI paint 
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		if (wParam == SIZE_RESTORED) {
			xwin_app::instance->on_resize(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(xwin_app::instance->get_hwnd());
		xwin_app::instance->on_mouse_button_down(MOUSE_BUTTON_LEFT, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		xwin_app::instance->on_mouse_button_up(MOUSE_BUTTON_LEFT, LOWORD(lParam), HIWORD(lParam));
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		xwin_app::instance->on_mouse_move(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_KEYDOWN:
		xwin_app::instance->on_key_down(wParam);
		break;
	case WM_DESTROY:
		xwin_app::instance->on_close();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

