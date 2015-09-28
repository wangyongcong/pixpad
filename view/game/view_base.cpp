#include "stdafx.h"

#include "view_base.h"
#include "view_sparrow.h"
#include "view_ogl3.h"

namespace wyc
{

	static const wchar_t* view_name[VIEW_TYPE_COUNT] =
	{
		L"Sparrow",
		L"OpenGL3",
	};

	std::shared_ptr<CViewBase> CViewBase::create_view(EViewType type, int x, int y, unsigned w, unsigned h)
	{
		std::shared_ptr<CViewBase> ptr_view = nullptr;
		switch (type)
		{
		case VIEW_SPARROW:
			ptr_view = std::shared_ptr<CViewBase>(new CViewSparrow);
			break;
		case VIEW_OPENGL3: {
			ptr_view = std::shared_ptr<CViewBase>(new CViewOgl3);
			break;
		}
		default:
			ptr_view = std::shared_ptr<CViewBase>(new CViewOgl3);
			break;
		}
		if (!ptr_view->initialize(x, y, w, h))
		{
			ptr_view = nullptr;
			return 0;
		}
		ptr_view->set_text(view_name[type]);
		return ptr_view;
	}
}
