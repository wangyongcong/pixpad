#ifndef WYC_HEADER_APP_PIXPAD
#define WYC_HEADER_APP_PIXPAD

#include "win_app.h"
#include "glrender.h"
#include "surface.h"

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
	private:
		GLuint m_tex;
		GLuint m_prog;
		GLuint m_vbo;
		GLuint m_ibo;
		xsurface m_surf;
		bool m_redraw;
	};

} // namespace wyc

#endif // WYC_HEADER_APP_PIXPAD
