#ifndef WYC_HEADER_RENDER
#define WYC_HEADER_RENDER

#ifndef GLEW_MX
#define GLEW_MX // enable multiple render context
#endif 
#include "GL/glew.h"
#include "GL/wglew.h"

#define GL_COMPAT

extern GLEWContext* glewGetContext();
extern WGLEWContext* wglewGetContext();

namespace wyc
{

HWND gl_create_window(HINSTANCE hInstance, HWND hParent, const wchar_t *name, int x, int y, unsigned width, unsigned height);
bool gl_detect_drivers(HWND hTmpWnd, const char *profile);
bool gl_create_context();

}; // namespace wyc

#endif // WYC_HEADER_RENDER
