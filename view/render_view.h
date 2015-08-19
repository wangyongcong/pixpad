#pragma once

namespace wyc
{
	enum view_type
	{
		VIEW_SOFT = 0,
		VIEW_OPEN_GL,
	};

	class render_view
	{
	public:
		static render_view* create_view(view_type type, int x, int y, unsigned w, unsigned h);

		virtual ~render_view() {}

	};


} // namespace wyc