#ifndef WYC_HEADER_GL_RENDER
#define WYC_HEADER_GL_RENDER

#ifndef GLEW_MX
#define GLEW_MX // enable multiple render context
#endif 
#include "GL/glew.h"
#include "GL/wglew.h"

#define GL_COMPATIBLE // Compatible with OpenGL 3.0 before

#define thread_local __declspec(thread)

// Interface for GLEW multi-thread context
// GLEW will use these interfaces to get context of current thread
extern GLEWContext* glewGetContext();
extern WGLEWContext* wglewGetContext();

namespace wyc
{
// class OpenGL context
class xglcontext;

// Get OpenGL version
void gl_get_version(int &major, int &minor);
// Get GLSL version
void glsl_get_version(int &major, int &minor);
// Create OpenGL target window
HWND gl_create_window(HINSTANCE hInstance, HWND hParent, int x, int y, unsigned width, unsigned height);
// Check OpenGL drivers
int  gl_detect_drivers(HWND hWnd, const char *profile = 0);
// Create OpenGL context
xglcontext* gl_create_context(HWND hWnd, int pixel_fmt);
// Destroy current context
void gl_destroy_context();
// Get current context
xglcontext* gl_get_context();

class xglcontext
{
public:
	inline HWND get_window() const {
		return m_hwnd;
	}
	inline HGLRC get_render_context() const {
		return m_hrc;
	}
	inline void swap_buffers() const {
		SwapBuffers(m_hdc);
	}
private:
	friend xglcontext* gl_create_context(HWND, int);
	friend void gl_destroy_context();
	friend GLEWContext* glewGetContext();
	friend WGLEWContext* wglewGetContext();
	
	xglcontext();
	bool create(HWND hwnd, int pixel_fmt);
	void destroy();

	HWND m_hwnd;
	HDC m_hdc;
	HGLRC m_hrc;
	GLEWContext m_glew_ctx;
	WGLEWContext m_wglew_ctx;

}; // class xglcontext

}; // namespace wyc

#endif // WYC_HEADER_GL_RENDER
