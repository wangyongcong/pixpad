#ifndef WYC_HEADER_APP_PIXPAD
#define WYC_HEADER_APP_PIXPAD

#include <vector>
#include <OpenEXR/ImathRandom.h>

#include "win_app.h"
#include "glrender.h"
#include "surface.h"
#include "mathfwd.h"

namespace wyc
{
	class xapp_pixpad : public xwin_app
	{
	public:
		virtual void on_start();
		virtual void on_close();
		virtual void on_render();
		virtual void on_key_down(int keycode);
		void on_paint();
		void random_triangle(int lx, int ly, int rx, int ry);
		void draw_cube(int lx, int ly, int rx, int ry);
		void draw_triangles(const std::vector<vec4f> &vertices);
	private:
		GLuint m_tex;
		GLuint m_prog;
		GLuint m_vbo;
		GLuint m_ibo;
		xsurface m_surf;
		bool m_redraw;
		Imath::Rand32 m_rnd;
	};

} // namespace wyc

#endif // WYC_HEADER_APP_PIXPAD
