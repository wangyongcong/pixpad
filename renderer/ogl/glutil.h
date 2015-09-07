#ifndef WYC_HEADER_GL_RENDER
#define WYC_HEADER_GL_RENDER

#ifndef GLEW_MX
#define GLEW_MX // enable multiple render context
#endif 
#include "GL/glew.h"
#include "GL/wglew.h"

#define GL_COMPATIBLE // Compatible with OpenGL 3.0 before

// Interface for GLEW multi-thread context
// GLEW will use these interfaces to get context of current thread
extern GLEWContext* glewGetContext();
extern WGLEWContext* wglewGetContext();

namespace wyc
{
// class OpenGL context
class xgl_context;

// Get OpenGL version
void gl_get_version(int &major, int &minor);
// Get GLSL version
void glsl_get_version(int &major, int &minor);
// Check OpenGL error
void gl_check_error(const char *tag=nullptr);
// Create OpenGL target window
HWND gl_create_window(HINSTANCE hInstance, HWND hParent, int x, int y, unsigned width, unsigned height);
// Check OpenGL drivers
int  gl_detect_drivers(HWND hWnd, const char *profile = 0);
// Create OpenGL context
xgl_context* gl_create_context(HWND hWnd, int pixel_fmt);
// Destroy current context
void gl_destroy_context();
// Get current context
xgl_context* gl_get_context();
// Load shader source
GLuint glsl_load_source(GLenum shader_type, const char *src, size_t length=0);
// Load shader source file
GLuint glsl_load_file(GLenum shader_type, const char *file_path);
// Build shader program
GLuint glsl_build_shader(GLuint *shaders, size_t count);

class xgl_context
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
	friend GLEWContext* ::glewGetContext();
	friend WGLEWContext* ::wglewGetContext();
	friend xgl_context* gl_create_context(HWND, int);
	friend void gl_destroy_context();
	
	xgl_context();
	bool create(HWND hwnd, int pixel_fmt);
	void destroy();

	HWND m_hwnd;
	HDC m_hdc;
	HGLRC m_hrc;
	GLEWContext m_glew_ctx;
	WGLEWContext m_wglew_ctx;

}; // class xgl_context

}; // namespace wyc

#endif // WYC_HEADER_GL_RENDER
