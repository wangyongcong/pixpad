#pragma once
#include "game_framework.h"
#include "game_window.h"
#include "util_macros.h"

namespace wyc
{
	class GAME_FRAMEWORK_API WindowsWindow : public IGameWindow
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(WindowsWindow)
	public:
		WindowsWindow();
		~WindowsWindow() override;

		// Implement IGameWinodw
		bool create(const wchar_t* title, uint32_t width, uint32_t height) override;
		void set_visible(bool is_visible) override;
		bool is_valid() const override;
		void get_window_size(unsigned& width, unsigned& height) const override;
		// IGameWindow 

		inline HWND GetWindowHandle() const
		{
			return m_hwnd;
		}

	protected:
		HWND m_hwnd;
		unsigned m_width, m_height;
	};
}
