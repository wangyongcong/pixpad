#include <string>
#include "application.h"
#include "game.h"

namespace wyc
{
	class windows_application : public application
	{
	public:
		bool initialize(const std::wstring &app_name, game* game_inst, HINSTANCE hInstance, size_t width=0, size_t height=0, LPTSTR cmd_line=nullptr);
		
		// application interface
		virtual void start();
		virtual void close();
		virtual void resize(unsigned view_w, unsigned view_h);
		virtual void on_event(void *ev);
		virtual bool on_command(int cmd_id, int cmd_event) { return false; }
		
		inline HINSTANCE os_instance() const
		{
			return m_hinst;
		}
		inline HWND os_window() const
		{
			return m_hwnd_main;
		}
		virtual const std::wstring& name() const
		{
			return m_app_name;
		}
		virtual void get_window_size(size_t &width, size_t &height) const
		{
			width = m_view_w;
			height = m_view_h;
		}

	protected:
		HINSTANCE m_hinst = NULL;
		HWND m_hwnd_main = NULL;
		std::wstring m_app_name;
		size_t m_view_w = 0, m_view_h = 0;
		game *m_game = 0;
	};

}; // end of namespace wyc
