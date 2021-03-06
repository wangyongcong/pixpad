#ifndef WYC_HEADER_APP_PIXPAD
#define WYC_HEADER_APP_PIXPAD

#include <vector>
#include <atomic>
#include <thread>
#include <array>

#include <OpenEXR/ImathRandom.h>

#include "game.h"
#include "view_base.h"
#include "renderer.h"
#include <renderer/scene.h>

namespace wyc
{
	class CGamePixpad : public CGame
	{
	public:
		CGamePixpad();
		virtual void on_start();
		virtual void on_close();
		virtual void on_update();
		virtual void on_resize(unsigned width, unsigned height) {};
		virtual bool is_exit();

		// input event handler
		virtual void on_mouse_button_down(EMouseButton button, int x, int y) {};
		virtual void on_mouse_button_up(EMouseButton button, int x, int y) {};
		virtual void on_mouse_move(int x, int y) {};
		virtual void on_mouse_wheel(int delta) {};
		virtual void on_key_down(int keycode);
		virtual void on_key_up(int keycode) {};

		virtual const std::wstring& name() const
		{
			return m_game_name;
		}

	private:
		void create_views();
		void start_task();

		uint64_t m_frame;
		using duration_t = std::chrono::milliseconds;
		duration_t m_total_time;
		std::array<duration_t, 240> m_frame_time;

		std::atomic_bool m_signal_exit;
		const std::wstring &m_game_name;
		std::vector<std::thread> m_thread_pool;
		std::vector<std::shared_ptr<CViewBase>> m_views;

		Imath::Rand32 m_rnd;
		CScene m_scene;
	};

} // namespace wyc

#endif // WYC_HEADER_APP_PIXPAD
