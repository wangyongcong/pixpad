#pragma once

#include <string>

namespace wyc
{
	enum view_type
	{
		VIEW_SPARROW = 0,
		VIEW_OPENGL3,

		VIEW_TYPE_COUNT,
	};

	class view_base
	{
	public:
		static view_base* create_view(view_type type, int x, int y, unsigned w, unsigned h);
		virtual ~view_base() {}
		virtual void set_text(const wchar_t *text) = 0;
		virtual void on_render() = 0;
	};


} // namespace wyc