#pragma once

#include <string>
#include <OpenEXR/ImathVec.h>
#include "view_base.h"

namespace wyc
{
	class CViewOgl3 : public CViewBase
	{
	public:
		CViewOgl3();
		~CViewOgl3() {}
		virtual bool initialize(int x, int y, unsigned w, unsigned h) override;
		virtual void suspend() override;
		virtual void wake_up() override;
		virtual void refresh() override;
		virtual void set_text(const wchar_t *text) override;
		virtual void on_render() override;
		virtual void get_position(int &x, int &y) override;
		virtual void get_size(unsigned &width, unsigned &height) override;
		virtual std::shared_ptr<CRenderer> get_renderer() override {
			return nullptr;
		}

	protected:
		HWND m_hwnd;
		int m_pixel_fmt;
		Imath::V2i m_view_pos;
		Imath::V2i m_view_size;
	};

} // namespace wyc