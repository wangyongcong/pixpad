#include "stdafx.h"
#include <clocale>
#include "app_pixpad.h"

wyc::application *g_application = nullptr;

wyc::application* wyc::application::get_instance()
{
	return g_application;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	using namespace wyc;
	std::wstring application_name = L"Pixpad";
	xapp_pixpad *app = new xapp_pixpad();
	if (!app->initialize(application_name, hInstance, 1279, 720, lpCmdLine))
		return 1;
	g_application = app;
	g_application->start();
	return 0;
}

