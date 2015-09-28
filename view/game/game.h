#pragma once
#include <string>
#include "input_event.h"

namespace wyc
{
	class CGame
	{
	public:
		virtual ~CGame() {}
		
		// application event handler
		virtual void on_start() = 0;
		virtual void on_close() = 0;
		virtual void on_update() = 0;
		virtual void on_resize(unsigned width, unsigned height) = 0;
		virtual bool is_exit() = 0;

		// input event handler
		virtual void on_mouse_button_down(EMouseButton button, int x, int y) = 0;
		virtual void on_mouse_button_up(EMouseButton button, int x, int y) = 0;
		virtual void on_mouse_move(int x, int y) = 0;
		virtual void on_mouse_wheel(int delta) = 0;
		virtual void on_key_down(int keycode) = 0;
		virtual void on_key_up(int keycode) = 0;

		virtual const std::wstring& name() const = 0;

	};
} // namespace wyc