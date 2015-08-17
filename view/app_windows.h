#include <string>
#include "application.h"

namespace wyc
{

	enum MOUSE_BUTTON
	{
		MOUSE_BUTTON_LEFT,
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_MIDDLE,
	};

	class windows_app : public application
	{
	public:
		bool initialize(const std::wstring &application_name, HINSTANCE hInstance, size_t width=0, size_t height=0, LPTSTR cmd_line=nullptr);
		// application interface
		virtual void start();
		virtual void close();
		virtual void update() {}
		virtual void render() {}
		virtual void resize(unsigned view_w, unsigned view_h);
		virtual void on_event(void *ev);
		virtual void on_start() {}
		virtual void on_close() {}
		virtual void on_render() {}
		virtual bool on_command(int cmd_id, int cmd_event) { return false; }
		
		// windows input handlers
		virtual void on_resize(unsigned width, unsigned height);
		virtual void on_mouse_button_down(MOUSE_BUTTON button, int x, int y) {}
		virtual void on_mouse_button_up(MOUSE_BUTTON button, int x, int y) {}
		virtual void on_mouse_move(int x, int y) {}
		virtual void on_mouse_wheel(int delta) {}
		virtual void on_key_down(int keycode) {}
		virtual void on_key_up(int keycode) {}

		inline HINSTANCE get_hinst() const
		{
			return m_hinst;
		}
		inline HWND get_hwnd() const
		{
			return m_hwnd_main;
		}
		inline const std::wstring& application_name() const
		{
			return m_app_name;
		}
		inline void get_viewport_size(size_t &width, size_t &height) const
		{
			width = m_view_w;
			height = m_view_h;
		}

	protected:
		HINSTANCE m_hinst = NULL;
		HWND m_hwnd_main = NULL;
		std::wstring m_app_name;
		size_t m_view_w, m_view_h;
	};

}; // end of namespace wyc
