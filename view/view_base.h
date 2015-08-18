#pragma once

namespace wyc
{
	class view_base
	{
	public:
		virtual ~view_base() {}
		virtual bool create(HWND parent, int x, int y, unsigned w, unsigned h) = 0;

	};

} // namespace wyc