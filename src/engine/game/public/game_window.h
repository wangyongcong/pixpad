#pragma once

namespace wyc
{
	class GAME_FRAMEWORK_API IGameWindow
	{
	public:
		virtual ~IGameWindow() = default;
		virtual bool create(const wchar_t* title, uint32_t width, uint32_t height) = 0;
		virtual void set_visible(bool bIsVisible) = 0;
		virtual bool is_valid() const = 0;
		virtual void get_window_size(unsigned &width, unsigned &height) const = 0;
	};
} // namespace wyc
