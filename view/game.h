#pragma once
#include <string>
#include "input_event.h"

namespace wyc
{
	class game
	{
	public:
		virtual ~game() {}
		
		// application event handler
		virtual void on_start() = 0;
		virtual void on_close() = 0;
		virtual void on_render() = 0;
		virtual void on_update() = 0;
		virtual void on_resize(unsigned width, unsigned height) = 0;

		// input event handler
		virtual void on_mouse_button_down(MOUSE_BUTTON button, int x, int y) = 0;
		virtual void on_mouse_button_up(MOUSE_BUTTON button, int x, int y) = 0;
		virtual void on_mouse_move(int x, int y) = 0;
		virtual void on_mouse_wheel(int delta) = 0;
		virtual void on_key_down(int keycode) = 0;
		virtual void on_key_up(int keycode) = 0;

		virtual const std::wstring& name() const = 0;

	};
} // namespace wyc