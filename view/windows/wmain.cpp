#include "stdafx.h"
#include "windows_application.h"
#include "game_pixpad.h"

namespace wyc
{
	application *g_application = nullptr;

	application* application::get_instance()
	{
		return g_application;
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	using namespace wyc;
	std::wstring application_name = L"Pixpad";
	windows_application *app = new windows_application();
	g_application = app;
	if (!app->initialize(application_name, new game_pixpad(), hInstance, 1280, 720, lpCmdLine))
		return 1;
	g_application->start();
	return 0;
}

