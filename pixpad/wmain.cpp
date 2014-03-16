#include "stdafx.h"
#include <cstdio>
#include <tuple>
#include "log.h"
#include "render.h"

#ifdef _DEBUG
	#pragma comment (lib, "mathexd.lib")
	#pragma comment (lib, "elogd.lib")
#else
	#pragma comment (lib, "mathex.lib")
	#pragma comment (lib, "elog.lib")
#endif 

struct AppContext
{
	HINSTANCE instance = NULL;
	HWND main_wnd = NULL;
	bool is_size_changed = false;
} g_app;

// Global logger
wyc::xlogger *g_log = NULL;
// Timer ID for log flushing
#define ID_TIMER_LOG 1

const wchar_t *APP_TITLE = L"Pixpad";		// app title
const wchar_t *WND_CLASS = L"PixpadMain";	// main window class name

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				OnResizeWindow (int, int);
void CALLBACK		TimerFlushLog(HWND, UINT, UINT_PTR, DWORD);
void				OnRender();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	// init logger
	g_log = new wyc::xlogger();
	g_log->create("pixpad",NULL,0,wyc::LOG_DEBUG);
	debug("Pixpad starting...");

	MSG msg;
	HACCEL hAccelTable = NULL;

	// register main window
	MyRegisterClass(hInstance);
	
	// init instance
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// main loop
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				break;
			}
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			OnRender();
			Sleep(1);
		}
	}
	debug("Pixpad exit");

	// release logger
	delete g_log;
	g_log = NULL;

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= (HICON)::LoadImage(NULL,L"favicon32.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName  = WND_CLASS;
	wcex.hIconSm		= (HICON)::LoadImage(NULL,L"favicon16.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   g_app.instance = hInstance;

   hWnd = CreateWindow(WND_CLASS, APP_TITLE, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_app.main_wnd = hWnd;

   RECT rectClient;
   GetClientRect(hWnd, &rectClient);
   int width = rectClient.right - rectClient.left;
   int height = rectClient.bottom - rectClient.top;
   // Create a temporary window to initialize driver
   HWND hTmpWnd = wyc::gl_create_window(hInstance, hWnd, 0, 0, width, height);
   int pixel_format = wyc::gl_detect_drivers(hTmpWnd);
   DestroyWindow(hTmpWnd);
   if (!pixel_format)
   {
	   return FALSE;
   }
   // Create the real target window
   HWND hTargetWnd = wyc::gl_create_window(hInstance, hWnd, 0, 0, width, height);
   if (!wyc::gl_create_context(hTargetWnd, pixel_format))
   {
	   return FALSE;
   }
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

   // Set viewport & background color
   glViewport(0, 0, width, height);
   glClearColor(0, 0, 0, 1.0f);

   // Set timer for log flushing 
   SetTimer(hWnd, ID_TIMER_LOG, 1000, &TimerFlushLog);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
// Main window
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	//PAINTSTRUCT ps;
	//HDC hdc;

	switch (message)
	{
	case WM_COMMAND: // Window menu
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	//case WM_PAINT:
	//	hdc = BeginPaint(hWnd, &ps);
	//	// TODO: custom GUI paint 
	//	EndPaint(hWnd, &ps);
	//	break;
	case WM_SIZE:
		if (wParam == SIZE_RESTORED) {
			OnResizeWindow(LOWORD(lParam),HIWORD(lParam));
			g_app.is_size_changed = true;
		}
		break;
	case WM_EXITSIZEMOVE:
		if (g_app.is_size_changed) {
			g_app.is_size_changed = false;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
// About dialog 
//
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

void OnResizeWindow(int width, int height)
{
	debug("resize window: %d x %d", width, height);
	assert(width > 0 && height > 0);
	HWND target_wnd = wyc::gl_get_context()->get_window();
	MoveWindow(target_wnd, 0, 0, width, height, FALSE);
	// TODO: rebuild OpenGL view
	glViewport(0, 0, width, height);
}

// Flush logger on time
void CALLBACK TimerFlushLog(HWND hwnd, UINT umsg, UINT_PTR id, DWORD time)
{
	if (id == ID_TIMER_LOG)
		g_log->flush();
}

// Render frame
void OnRender()
{
	glClear(GL_COLOR_BUFFER_BIT);
	wyc::gl_get_context()->swap_buffers();
}
