#include "stdafx.h"
#include <cassert>
#include <ctime>
#include <fstream>
#include "log.h"
#include "render.h"

namespace wyc
{

// OpenGL & GLSL version
static int GL_VERSION_MAJOR = 0;
static int GL_VERSION_MINOR = 0;
static int GLSL_VERSION_MAJOR = 0;
static int GLSL_VERSION_MINOR = 0;

// GL context instance of current thread
thread_local xglcontext *tls_gl_context = NULL;

// function pointer to wglCreateContextAttribsARB
static PFNWGLCREATECONTEXTATTRIBSARBPROC create_context_attribs = 0;
// function pointer to wglChoosePixelFormatARB
static PFNWGLCHOOSEPIXELFORMATARBPROC choose_pixel_format = 0;

#pragma warning(push)
#pragma warning(disable:4996)
void _parse_ver_str(const char *ver, int &major, int &minor)
{
	size_t sz = strlen(ver);
	char *pstr = new char[sz + 1];
	strcpy(pstr, ver);
	const char *splitter = ".";
	char *tok = strtok(pstr, splitter);
	if (tok) {
		major = atoi(tok);
		tok = strtok(0, splitter);
		if (tok) minor = atoi(tok);
	}
	delete[] pstr;
}
#pragma warning(pop)

void gl_get_version(int &major, int &minor)
{
	if (!GL_VERSION_MAJOR) {
		const char *version = (const char*)glGetString(GL_VERSION);
		_parse_ver_str(version, GL_VERSION_MAJOR, GL_VERSION_MINOR);
	}
	major = GL_VERSION_MAJOR;
	minor = GL_VERSION_MINOR;
}

void glsl_get_version(int &major, int &minor)
{
	if (!GLSL_VERSION_MAJOR)
	{
		const char *glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		_parse_ver_str(glsl_version, GLSL_VERSION_MAJOR, GLSL_VERSION_MINOR);
	}
	major = GLSL_VERSION_MAJOR;
	minor = GLSL_VERSION_MINOR;
}

HWND gl_create_window(HINSTANCE hInstance, HWND hParent, int x, int y, unsigned width, unsigned height)
{
	assert(hInstance);
	const wchar_t *className = L"OGLTargetWindow";
	WNDCLASSEX wndcls;
	if (!GetClassInfoEx(hInstance, className, &wndcls)) {
		wndcls.cbSize = sizeof(WNDCLASSEX);
		wndcls.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndcls.hInstance = hInstance;
		wndcls.lpfnWndProc = WNDPROC(&DefWindowProc);
		wndcls.cbClsExtra = 0;
		wndcls.cbWndExtra = 0;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hIcon = NULL;
		wndcls.hIconSm = NULL;
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = className;
		if (!RegisterClassEx(&wndcls))
			return NULL;
	}
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if (NULL != hParent)
		style |= WS_CHILDWINDOW;
	return CreateWindowEx(0, className, NULL, style,
		x, y, width, height, hParent, NULL, hInstance, NULL);
}

int gl_detect_drivers(HWND hWnd, const char *profile)
{
	if (NULL == hWnd)
		return 0;
	HDC hDC = ::GetDC(hWnd);
	if (NULL == hDC)
		return 0;
	// 最简单的PFD, 避免出错
	::PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_COMPOSITION |			// Wordk with DWM 
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
		32,									// 32-bit Color Depth
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored
		8,									// 8-bit alpha
		0,									// Shift Bit Ignored
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		0,									// 0-bit Z-Buffer (Depth Buffer)  
		0,									// 0-bit Stencil Buffer
		0,									// No Auxiliary Buffer
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved
		0, 0, 0								// layer masks ignored 
	};
	if (!SetPixelFormat(hDC, 1, &pfd))
		return 0;
	HGLRC hRC = wglCreateContext(hDC);
	if (NULL == hRC)
		return 0;
	wglMakeCurrent(hDC, hRC);
	int pixel_fmt = 0;
	unsigned cnt_fmt;
	int major, minor;
	gl_get_version(major, minor);
	if (major < 3)
	{
		error("Need OpenGL 3.0 or later");
		goto EXIT;
	}
	choose_pixel_format = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (choose_pixel_format == 0) {
		error("Not support WGL_ARB_pixel_format");
		goto EXIT;
	}
	create_context_attribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (create_context_attribs == 0){
		error("Failed to get wglCreateContextAttribsARB");
		goto EXIT;
	}
	int attribs[] = {
		WGL_SUPPORT_OPENGL_ARB, 1,	// Must support OGL rendering
		WGL_DRAW_TO_WINDOW_ARB, 1,	// pf that can run a window
		WGL_RED_BITS_ARB, 8,	// bits of red precision in window
		WGL_GREEN_BITS_ARB, 8,	// bits of green precision in window
		WGL_BLUE_BITS_ARB, 8,	// bits of blue precision in window
		WGL_ALPHA_BITS_ARB, 8,	// bits of alpha precision in window
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // pf should be RGBA type
		WGL_DEPTH_BITS_ARB, 24,	// bits of depth precision for window
		WGL_STENCIL_BITS_ARB, 8,	// bits of precision for stencil buffer
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // must be HW accelerated
		WGL_DOUBLE_BUFFER_ARB, 1, // double buffer enabled
		WGL_SAMPLES_ARB, 0, // number of multisample samples per pixel
		0 // NULL termination
	};
	if (!choose_pixel_format(hDC, attribs, 0, 1, &pixel_fmt, &cnt_fmt) || pixel_fmt == -1) {
		fatal("Can't find a proper pixel format");
		goto EXIT;
	}
EXIT:
	wglMakeCurrent(hDC, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
	return pixel_fmt;
}

xglcontext* gl_create_context(HWND hWnd, int pixel_fmt)
{
	if (tls_gl_context)
	{
		return tls_gl_context;
	}
	// create OpenGL context
	xglcontext *glctx = new xglcontext();
	if (!glctx->create(hWnd, pixel_fmt))
	{
		delete glctx;
		return 0;
	}
	tls_gl_context = glctx;
	// Initialize OpenGL extensions 
	GLenum err;
	err = glewInit();
	if (GLEW_OK != err) 
	{
		warn("Failed to init GL extensions: %s", (const char*)glewGetErrorString(err));
	}
	err = wglewInit();
	if (GLEW_OK != err) 
	{
		warn("Failed to WGL extensions: %s", (const char*)glewGetErrorString(err));
	}
	return glctx;
}

void gl_destroy_context()
{
	if (tls_gl_context != NULL) {
		tls_gl_context->destroy();
		tls_gl_context = NULL;
	}
}

xglcontext* gl_get_context()
{
	return tls_gl_context;
}

xglcontext::xglcontext()
{
	m_hwnd = NULL;
	m_hdc = NULL;
	m_hrc = NULL;
}

bool xglcontext::create(HWND hwnd, int pixel_fmt)
{
	if (NULL == hwnd)
		return false;
	HDC hdc = GetDC(hwnd);
	HGLRC hrc = NULL;
	assert(hdc != NULL);
	PIXELFORMATDESCRIPTOR pfd;
	if (!SetPixelFormat(hdc, pixel_fmt, &pfd)) {
		ReleaseDC(hwnd, hdc);
		error("Can't set pixel format: %d",pixel_fmt);
		return false;
	}
	if (create_context_attribs) {
		// OpenGL 3.0 or later
		int attributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, GL_VERSION_MAJOR,
			WGL_CONTEXT_MINOR_VERSION_ARB, GL_VERSION_MINOR,
#ifndef GL_COMPATIBLE
			// 只兼容OpenGL Core, 去除所有deprecated, 适用于OpenGL 3.2 (或更高)
			// TODO：GLEW在该模式下不可用,因为glewGetExtension使用旧的
			// glGetString(GL_EXTENSIONS) 来获取扩展的名字
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#else
			// 兼容3.0之前的版本
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#endif // GL_COMPAT
#ifdef _DEBUG
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, // enable debug mode
#endif // _DEBUG
			0,
		};
		hrc = create_context_attribs(hdc, NULL, attributes);
	}
	else {
		hrc = ::wglCreateContext(hdc);
	}
	if (hrc == NULL) {
		ReleaseDC(hwnd, hdc);
		error("failed to create render context");
		return false;
	}
	wglMakeCurrent(hdc, hrc);
	m_hwnd = hwnd;
	m_hdc = hdc;
	m_hrc = hrc;
	return true;
}

void xglcontext::destroy()
{
	if (m_hrc) {
		wglMakeCurrent(m_hdc, NULL);
		wglDeleteContext(m_hrc);
		ReleaseDC(m_hwnd, m_hdc);
		m_hwnd = NULL;
		m_hdc = NULL;
		m_hrc = NULL;
	}
}

void glsl_print_error (GLuint shader)
{
	GLint ret;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret);
	if (ret<1) {
		error("Shader compile error: Unknown");
		return;
	}
	ret += 1;
	char *info = new char[ret];
	glGetShaderInfoLog(shader, ret, &ret, info);
	error("Shader compile error:\n%s", info);
	delete[] info;
}

GLuint glsl_load_source(GLenum shader_type, const char *src, size_t length)
{
	GLuint shader = glCreateShader(shader_type);
	if (shader == 0) {
		return 0;
	}
	const GLchar *src_list[1] = { src };
	const GLint len_list[1] = { length };
	glShaderSource(shader, 1, src_list, len_list);
	glCompileShader(shader);
	GLint ret;
	::glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
	if (ret == GL_FALSE) {
		glsl_print_error(shader);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint glsl_load_file(GLenum shader_type, const char *file_path)
{
	std::fstream fs;
	fs.open(file_path, std::ios_base::in);
	if (!fs.is_open()) {
		error("Can't open file: %s", file_path);
		return 0;
	}
	fs.seekg(0, std::ios_base::end);
	size_t size = fs.tellg();
	char *src = new char[size + 1];
	fs.seekg(0);
	fs.read(src, size);
	std::streamoff  cnt = fs.gcount();
	if (size<cnt) {
		fs.close();
		delete[] src;
		error("Read file error");
		return 0;
	}
	src[cnt] = 0;
	fs.close();
	GLuint shader = glsl_load_source(shader_type, src, size);
	delete[] src;
	return shader;
}

GLuint glsl_build_shader(GLuint *shaders, size_t count)
{
	GLuint program = glCreateProgram();
	for (size_t i = 0; i < count; ++i)
		glAttachShader(program, shaders[i]);
	glLinkProgram(program);
	GLint ret;
	::glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (ret == GL_FALSE) {
		glsl_print_error(program);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

}; // namespace wyc

// Inplement GLEW interface
GLEWContext* glewGetContext()
{
	return &wyc::tls_gl_context->m_glew_ctx;
}
WGLEWContext* wglewGetContext()
{
	return &wyc::tls_gl_context->m_wglew_ctx;
}