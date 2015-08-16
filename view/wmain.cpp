#include "stdafx.h"
#include <clocale>
#include "app_windows.h"

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
	g_application = windows_app::create_instance(application_name, hInstance, 1280, 720, lpCmdLine);
	if (!g_application)
		return 1;
	g_application->start();
	return 0;
}

