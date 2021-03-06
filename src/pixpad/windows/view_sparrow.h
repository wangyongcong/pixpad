#pragma once
#include "view_base.h"
#include "sparrow/spw_renderer.h"

namespace wyc
{
	class CViewSparrow : public CViewBase
	{
	public:
		CViewSparrow();
		virtual ~CViewSparrow();
		virtual bool initialize(int x, int y, unsigned w, unsigned h) override;
		virtual void suspend();
		virtual void wake_up();
		virtual void refresh();
		virtual void on_render() override;
		virtual void set_text(const wchar_t *text) override;
		virtual void get_position(int &x, int &y) override;
		virtual void get_size(unsigned &width, unsigned &height) override;
		virtual std::shared_ptr<CRenderer> get_renderer() override {
			return m_renderer;
		}

		// window thread: update window display
		void on_paint();
		// render thread: present current frame
		void present();

	protected:
		bool rebuild_resource();
		void discard_resource();

		HWND m_hwnd;
		ID2D1Factory *m_d2d_factory;
		ID2D1HwndRenderTarget *m_d2d_rt;
		ID2D1Bitmap *m_bitmap;
		std::shared_ptr<CSpwRenderer> m_renderer;
		std::shared_ptr<CSpwRenderTarget> m_target;
		Imath::V2i m_view_pos;
		Imath::V2i m_view_size;
		std::mutex m_suspend_lock;
		std::atomic<bool> m_suspended;
	};

} // namespace wyc