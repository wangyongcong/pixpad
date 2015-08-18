#ifndef WYC_HEADER_APP_PIXPAD
#define WYC_HEADER_APP_PIXPAD

#include <vector>
#include <OpenEXR/ImathRandom.h>

#include "app_windows.h"
#include "glrender.h"
#include "raster/surface.h"
#include "math/mathfwd.h"

namespace wyc
{
	class xapp_pixpad : public windows_app
	{
	public:
		virtual void on_start();
		virtual void on_close();
		virtual void render();
		virtual void update();
		virtual void on_key_down(int keycode);
		void on_paint();
		void random_triangle(int lx, int ly, int rx, int ry);
		void draw_cube(int lx, int ly, int rx, int ry);
		void draw_triangles(const std::vector<vec4f> &vertices);
	private:
		void create_view(unsigned view_count);
		void create_view_window(int x, int y, unsigned w, unsigned h);

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
