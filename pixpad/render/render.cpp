#include "stdafx.h"
#include "render.h"

namespace wyc
{

HWND gl_create_window(HINSTANCE hInstance, HWND hParent, const wchar_t *name, int x, int y, unsigned width, unsigned height)
{
	return NULL;
}

bool gl_detect_drivers(HWND hTmpWnd, const char *profile)
{
	return false;
}

bool gl_create_context()
{
	return false;
}

}; // namespace wyc