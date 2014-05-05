#include <string>

namespace wyc
{

	enum MOUSE_BUTTON
	{
		MOUSE_BUTTON_LEFT,
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_MIDDLE,
	};

	class xwin_app
	{
	public:
		static xwin_app *instance;
		static bool initialize(const std::wstring &app_name, HINSTANCE hInstance, LPTSTR cmd_line);
		static void destroy();
		virtual ~xwin_app() {}
		void run();
		virtual void on_start() {}
		virtual void on_close() {}
		virtual void on_render() {}
		virtual bool on_command(int cmd_id, int cmd_event) { return false; }
		virtual void on_resize(unsigned width, unsigned height) {}
		virtual void on_mouse_button_down(MOUSE_BUTTON button, int x, int y) {}
		virtual void on_mouse_button_up(MOUSE_BUTTON button, int x, int y) {}
		virtual void on_mouse_move(int x, int y) {}
		virtual void on_mouse_wheel(int delta) {}
		virtual void on_key_down(int keycode) {}
		virtual void on_key_up(int keycode) {}

		HINSTANCE get_hinst() const
		{
			return m_hinst;
		}
		HWND get_hwnd() const
		{
			return m_hwnd_main;
		}
		const std::wstring& app_name() const
		{
			return m_app_name;
		}

	protected:
		HINSTANCE m_hinst = NULL;
		HWND m_hwnd_main = NULL;
		std::wstring m_app_name;
	};

}; // end of namespace wyc
