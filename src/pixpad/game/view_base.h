#pragma once

#include <string>
#include <memory>
#include "renderer.h"

namespace wyc
{
	enum EViewType
	{
		VIEW_SPARROW = 0,
		VIEW_OPENGL3,

		VIEW_TYPE_COUNT,
	};

	class CViewBase
	{
	public:
		static std::shared_ptr<CViewBase> create_view(EViewType type, int x, int y, unsigned w, unsigned h);
		virtual ~CViewBase() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h) = 0;
		virtual void suspend() = 0;
		virtual void wake_up() = 0;
		virtual void refresh() = 0;
		virtual void set_text(const wchar_t *text) = 0;
		virtual void on_render() = 0;
		virtual void get_position(int &x, int &y) = 0;
		virtual void get_size(unsigned &width, unsigned &height) = 0;
		virtual std::shared_ptr<CRenderer> get_renderer() = 0;
	};


} // namespace wyc