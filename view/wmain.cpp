#include "stdafx.h"
#include <locale>
#include "app_pixpad.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	using namespace wyc;
	setlocale(LC_ALL, "chs");
	std::wstring app_name = L"Pixpad";
	xwin_app::instance = new xapp_pixpad();
	if (!xwin_app::initialize(app_name, hInstance, 800, 600, lpCmdLine))
		return 1;
	xwin_app::instance->run();
	xwin_app::destroy();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using namespace wyc;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
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
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
