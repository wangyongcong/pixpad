#include "stdafx.h"
#include "windows_application.h"
#include "game_pixpad.h"

namespace wyc
{
	CApplication *g_application = nullptr;

	CApplication* CApplication::get_instance()
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
	CWindowsApplication *app = new CWindowsApplication();
	g_application = app;
	if (!app->initialize(application_name, new CGamePixpad(), hInstance, 800, 600, lpCmdLine))
		return 1;
	g_application->start();
	return 0;
}

