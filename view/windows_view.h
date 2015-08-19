#pragma once

#include <string>
#include "render_view.h"

namespace wyc
{
	class windows_view : public render_view
	{
	public:
		windows_view();
		~windows_view() {}
		bool create(HWND parent, int x, int y, unsigned w, unsigned h);
		void set_text(const std::wstring &text);
		const std::wstring& get_text() const 
		{
			return m_text;
		}
	private:
		HWND m_hwnd;
		std::wstring m_text;
	};

} // namespace wyc