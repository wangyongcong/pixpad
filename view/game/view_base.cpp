#include "stdafx.h"
#include "view_base.h"
#include "view_ogl3.h"

namespace wyc
{

	static const wchar_t* view_name[VIEW_TYPE_COUNT] =
	{
		L"Sparrow",
		L"OpenGL3",
	};

	view_base * view_base::create_view(view_type type, int x, int y, unsigned w, unsigned h)
	{
		view_base *ptr_view = nullptr;
		switch (type)
		{
		case VIEW_SPARROW:
			ptr_view = new view_ogl3();
			break;
		case VIEW_OPENGL3: {
			ptr_view = new view_ogl3();
			((view_ogl3*)ptr_view)->create(NULL, x, y, w, h);
			break;
		}
		default:
			ptr_view = new view_ogl3();
			break;
		}
		ptr_view->set_text(view_name[type]);
		return ptr_view;
	}
}
