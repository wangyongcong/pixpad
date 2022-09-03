#include "engine_pch.h"
#include "resource.h"
#include "windows_platform.h"
#include "windows_window.h"
#include "common/log_macros.h"

namespace wyc
{
#define IS_WINDOW_VALID(Handle) ((Handle) != NULL)

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

	bool WindowsWindow::create(const wchar_t* title, uint32_t width, uint32_t height)
	{
		static const wchar_t* sWindowClassName = L"MainGameWindow";
		static bool sIsClassRegistered = false;

		if(!sIsClassRegistered)
		{
			WNDCLASSEX windowClass;
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = &WindowProcess;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = gAppInstance;

			HINSTANCE moduleInstance = GetModuleInstance();
			windowClass.hIcon = LoadIcon(moduleInstance, MAKEINTRESOURCE(IDI_APP_ICON));
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.hbrBackground = HBRUSH(COLOR_WINDOW);
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = sWindowClassName;
			windowClass.hIconSm = LoadIcon(moduleInstance, MAKEINTRESOURCE(IDI_APP_ICON));

			HRESULT hr = RegisterClassEx(&windowClass);
			if (!SUCCEEDED(hr))
			{
				// fail to register window
				LogError("Fail to register windows class %s", sWindowClassName);
				return nullptr;
			}
			sIsClassRegistered = true;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		RECT clientRect = { 0, 0, int(width), int(height) };
		AdjustWindowRect(&clientRect, style, FALSE);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

		HWND hMainWnd = CreateWindowW(sWindowClassName, title, style,
			windowX, windowY, windowWidth, windowHeight, NULL, NULL, gAppInstance, NULL);
		if (!hMainWnd)
		{
			// fail to create windows
			return false;
		}
		
		m_hwnd = hMainWnd;
		m_width = windowWidth;
		m_height = windowHeight;
		return true;
	}

	void WindowsWindow::set_visible(bool is_visible)
	{
		if(!IS_WINDOW_VALID(m_hwnd))
		{
			return;
		}
		if(is_visible)
		{
			ShowWindow(m_hwnd, SW_NORMAL);
		}
		else
		{
			ShowWindow(m_hwnd, SW_HIDE);
		}
	}

	bool WindowsWindow::is_valid() const
	{
		return IS_WINDOW_VALID(m_hwnd);
	}

	void WindowsWindow::get_window_size(unsigned& width, unsigned& height) const
	{
		width = m_width;
		height = m_height;
	}

	WindowsWindow::WindowsWindow()
		: m_hwnd(NULL)
		, m_width(0)
		, m_height(0)
	{

	}

	WindowsWindow::~WindowsWindow()
	{
		if(m_hwnd != NULL)
		{
			DestroyWindow(m_hwnd);
			m_hwnd = NULL;
		}
	}

}
