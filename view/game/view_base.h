#pragma once

#include <string>
#include <memory>

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
		static std::shared_ptr<view_base> create_view(view_type type, int x, int y, unsigned w, unsigned h);
		virtual ~view_base() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h) = 0;
		virtual void set_text(const wchar_t *text) = 0;
		virtual void on_render() = 0;
		virtual void get_position(int &x, int &y) = 0;
		virtual void get_size(unsigned &width, unsigned &height) = 0;
	};


} // namespace wyc