#ifndef WYC_HEADER_APP_PIXPAD
#define WYC_HEADER_APP_PIXPAD

#include <vector>
#include <atomic>
#include <thread>

#include <OpenEXR/ImathRandom.h>

#include "game.h"
#include "view_base.h"

namespace wyc
{
	class game_pixpad : public game
	{
	public:
		game_pixpad();
		virtual void on_start();
		virtual void on_close();
		virtual void on_update();
		virtual void on_resize(unsigned width, unsigned height) {};
		virtual bool is_exit();

		// input event handler
		virtual void on_mouse_button_down(MOUSE_BUTTON button, int x, int y) {};
		virtual void on_mouse_button_up(MOUSE_BUTTON button, int x, int y) {};
		virtual void on_mouse_move(int x, int y) {};
		virtual void on_mouse_wheel(int delta) {};
		virtual void on_key_down(int keycode);
		virtual void on_key_up(int keycode) {};

		virtual const std::wstring& name() const
		{
			return m_game_name;
		}

		//void on_paint();
		//void random_triangle(int lx, int ly, int rx, int ry);
		//void draw_cube(int lx, int ly, int rx, int ry);
		//void draw_triangles(const std::vector<vec4f> &vertices);
	private:
		void create_views();

		const std::wstring &m_game_name;
		std::vector<std::thread> m_thread_pool;
		std::atomic_bool m_signal_exit;

		//GLuint m_tex = 0;
		//GLuint m_prog = 0;
		//GLuint m_vbo = 0;
		//GLuint m_ibo = 0;
		//xsurface m_surf;
		//bool m_redraw = false;
		Imath::Rand32 m_rnd;
	};

} // namespace wyc

#endif // WYC_HEADER_APP_PIXPAD
