#pragma once
#include "view_base.h"
#include "sparrow/spw_renderer.h"

namespace wyc
{
	class view_sparrow : public view_base
	{
	public:
		view_sparrow();
		virtual ~view_sparrow();
		virtual bool initialize(int x, int y, unsigned w, unsigned h) override;
		virtual void on_render() override;
		virtual void set_text(const wchar_t *text) override;
		virtual void get_position(int &x, int &y) override;
		virtual void get_size(unsigned &width, unsigned &height) override;
		virtual std::shared_ptr<renderer> get_renderer() override {
			return m_renderer;
		}
	protected:
		bool rebuild_resource();
		void discard_resource();
		void present();

		HWND m_hwnd;
		ID2D1Factory *m_d2d_factory;
		ID2D1HwndRenderTarget *m_d2d_rt;
		ID2D1Bitmap *m_bitmap;
		std::shared_ptr<spw_renderer> m_renderer;
		std::shared_ptr<spw_render_target> m_target;
		Imath::V2i m_view_pos;
		Imath::V2i m_view_size;
	};

} // namespace wyc