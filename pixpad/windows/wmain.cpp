#include "stdafx.h"
#include "resource.h"
#include <cstdio>
#include <tuple>
#include "util/log.h"
#include "glrender.h"
#include "glpipeline.h"
#include "swpipeline.h"

#ifdef _DEBUG
	#pragma comment (lib, "mathexd.lib")
	#pragma comment (lib, "elogd.lib")
#else
	#pragma comment (lib, "mathex.lib")
	#pragma comment (lib, "elog.lib")
#endif 

enum RENDER_MODE {
	SOFTWARE_RENDERER = 0,
	OPENGL_RENDERER,

	RENDERER_COUNT
};

enum MODEL_ID {
	REGULAR_TRIANGLE = 0,
	SQUARE_PLANE,

	MODEL_COUNT,
};

struct AppContext
{
	HINSTANCE instance = NULL;
	HWND main_wnd = NULL;
	bool is_size_changed = false;
	// renderer
	wyc::xpipeline *pipelines[RENDERER_COUNT];
	RENDER_MODE render_mode = OPENGL_RENDERER;
	// models
	wyc::xmat4f_t transform;
	std::pair<wyc::xvertex_buffer*, wyc::xindex_buffer*> models[MODEL_COUNT];
	MODEL_ID model_id = REGULAR_TRIANGLE;
	std::string material;

	AppContext()
	{
		for (int i = 0; i < RENDERER_COUNT; ++i)
		{
			pipelines[i] = nullptr;
		}
		for (int i = 0; i < MODEL_COUNT; ++i) {
			models[i].first = nullptr;
			models[i].second = nullptr;
		}
		transform.identity();
	}

	~AppContext()
	{
		wyc::xpipeline *pipeline;
		for (int i = 0; i < RENDERER_COUNT; ++i)
		{
			pipeline = this->pipelines[i];
			if (pipeline) 
				delete pipeline;
		}
		for (int i = 0; i < MODEL_COUNT; ++i) {
			if (models[i].first) 
				delete models[i].first;
			if (models[i].second)
				delete models[i].second;
		}
	}

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
void				SetRenderMode(RENDER_MODE mode);

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

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIXPAD));

	// register main window
	MyRegisterClass(hInstance);
	
	// init instance
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// main loop
	MSG msg;
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIXPAD));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = HBRUSH(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PIXPAD);
	wcex.lpszClassName  = WND_CLASS;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	g_app.instance = hInstance;

	hWnd = CreateWindow(WND_CLASS, APP_TITLE, (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME,
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

	// Initialize menu items
	HMENU main_menu = GetMenu(g_app.main_wnd);
	CheckMenuRadioItem(main_menu, IDM_SOFTWARE, IDM_OPENGL, IDM_OPENGL, MF_BYCOMMAND);
	CheckMenuRadioItem(main_menu, IDM_REGULAR_TRIANGLE, IDM_REGULAR_TRIANGLE, IDM_REGULAR_TRIANGLE, MF_BYCOMMAND);

	// Show OpenGL infomation
	const char *version = (const char*)glGetString(GL_VERSION);
	const char *glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	const char *vendor = (const char*)glGetString(GL_VENDOR);
	const char *device = (const char*)glGetString(GL_RENDERER);
	info("%s %s", vendor, device);
	info("OpenGL %s (GLSL %s)", version, glsl_version);
	info("GLEW version %s", glewGetString(GLEW_VERSION));

	// initialize OpenGL
	glClearColor(0, 0, 0, 1.0f);
	
	// create models
	wyc::xvertex_buffer *vertices = new wyc::xvertex_buffer();
	wyc::xindex_buffer *indices = new wyc::xindex_buffer();
	g_app.models[g_app.model_id] = { vertices, indices };
	wyc::gen_regular_triangle<wyc::xvertex_p3c3>(1, *vertices, *indices);
	auto v = vertices->get_as<wyc::xvertex_p3c3>();
	v[0].color.set(1.0, 0, 0);
	v[1].color.set(0, 1.0, 0);
	v[2].color.set(0, 0, 1.0);

	g_app.material = "vertex_color";
	g_app.transform.identity();
	g_app.transform.set_col(3, wyc::xvec3f_t(0, 0, -4));

	// initialize renderer
	g_app.pipelines[SOFTWARE_RENDERER] = new wyc::xsw_pipeline();
	g_app.pipelines[OPENGL_RENDERER] = new wyc::xgl_pipeline();
	wyc::xpipeline *pipeline = g_app.pipelines[g_app.render_mode];
	if (pipeline) {
		auto model = g_app.models[g_app.model_id];
		pipeline->commit(model.first, model.second);
		pipeline->set_transform(g_app.transform);
		pipeline->set_material(g_app.material);
	}

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
		case IDM_OPENGL:
			CheckMenuRadioItem(GetMenu(g_app.main_wnd), IDM_SOFTWARE, IDM_OPENGL, IDM_OPENGL, MF_BYCOMMAND);
			SetRenderMode(OPENGL_RENDERER);
			break;
		case IDM_SOFTWARE:
			CheckMenuRadioItem(GetMenu(g_app.main_wnd), IDM_SOFTWARE, IDM_OPENGL, IDM_SOFTWARE, MF_BYCOMMAND);
			SetRenderMode(SOFTWARE_RENDERER);
			break;
		case IDM_ABOUT:
			DialogBox(g_app.instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
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
	glViewport(0, 0, width, height);
	wyc::xpipeline *pipeline = g_app.pipelines[g_app.render_mode];
	if (pipeline) {
		pipeline->set_viewport(width, height);
		pipeline->set_perspective(45, float(width) / height, 1, 1000);
	}
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	wyc::xpipeline *pipeline = g_app.pipelines[g_app.render_mode];
	if (pipeline)
		pipeline->render();
	wyc::gl_get_context()->swap_buffers();
}

void SetRenderMode(RENDER_MODE mode)
{
	if (mode == g_app.render_mode)
		return;
	g_app.render_mode = mode;
	wyc::xpipeline *pipeline = g_app.pipelines[mode];
	if (!pipeline)
		return;
	RECT rectClient;
	GetClientRect(g_app.main_wnd, &rectClient);
	int width = rectClient.right - rectClient.left;
	int height = rectClient.bottom - rectClient.top;
	pipeline->set_viewport(width, height);
	pipeline->set_perspective(45, float(width) / height, 1, 1000);

	auto model = g_app.models[g_app.model_id];
	pipeline->commit(model.first, model.second);
	pipeline->set_transform(g_app.transform);
	pipeline->set_material(g_app.material);
}