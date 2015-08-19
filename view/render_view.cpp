#include "stdafx.h"
#include "render_view.h"
#include "windows_view.h"

namespace wyc
{
	render_view * render_view::create_view(view_type type, int x, int y, unsigned w, unsigned h)
	{
		render_view *ptr_view = nullptr;
		switch (type)
		{
		case VIEW_SOFT:
			ptr_view = new windows_view();
			break;
		case VIEW_OPEN_GL:
			ptr_view = new windows_view();
			break;
		default:
			ptr_view = new windows_view();
			break;
		}
		return ptr_view;
	}
}
