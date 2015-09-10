#include "stdafx.h"
#include "view_sparrow.h"

#include <thread>

#include "log.h"
#include "ogl/glutil.h"
#include "windows_application.h"

#define SAFE_RELEASE(ptr) if(ptr) {ptr->Release(); ptr=nullptr;}

namespace wyc
{
	view_sparrow::view_sparrow() : 
		m_hwnd(NULL), 
		m_d2d_factory(nullptr), 
		m_d2d_rt(nullptr), 
		m_bitmap(nullptr), 
		m_renderer(nullptr)
	{
	}

	view_sparrow::~view_sparrow()
	{
		m_hwnd = NULL;
		discard_resource();
		SAFE_RELEASE(m_d2d_factory);
		m_renderer = nullptr;
		m_target = nullptr;
	}

	bool view_sparrow::initialize(int x, int y, unsigned w, unsigned h)
	{
		windows_application* app_inst = dynamic_cast<windows_application*>(application::get_instance());
		if (!app_inst)
		{
			return false;
		}

		HINSTANCE os_instance = app_inst->os_instance();
		HWND main_wnd = app_inst->os_window();

		const wchar_t *className = L"D2DTargetWindow";
		WNDCLASSEX wndcls;
		if (!GetClassInfoEx(os_instance, className, &wndcls)) {
			wndcls.cbSize = sizeof(WNDCLASSEX);
			wndcls.style = CS_HREDRAW | CS_VREDRAW;
			wndcls.hInstance = os_instance;
			wndcls.lpfnWndProc = WNDPROC(&DefWindowProc);
			wndcls.cbClsExtra = 0;
			wndcls.cbWndExtra = 0;
			wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
			wndcls.hIcon = NULL;
			wndcls.hIconSm = NULL;
			wndcls.hbrBackground = NULL;
			wndcls.lpszMenuName = NULL;
			wndcls.lpszClassName = className;
			if (!RegisterClassEx(&wndcls))
				return false;
		}

		HWND target_wnd = CreateWindowEx(0, className, NULL, WS_CHILDWINDOW,
			x, y, w, h, main_wnd, NULL, os_instance, NULL);
		if (!target_wnd)
		{
			return false;
		}

		ID2D1Factory *ptr_factory = nullptr;
		D2D1_FACTORY_OPTIONS options;
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &ptr_factory);
		if (result != S_OK)
		{
			error("Failed to initialize Direct2D factory.");
			return false;
		}

		m_hwnd = target_wnd;
		m_d2d_factory = ptr_factory;

		if (!rebuild_resource())
		{
			error("Failed to create graphic resource.");
			return false;
		}
		
		m_target = std::make_shared<spr_render_target>();
		if (!m_target->create(w, h, SPR_COLOR_B8G8R8A8 | SPR_DEPTH_16))
		{
			m_target = nullptr;
			error("Failed to create sparrow render target.");
			return false;
		}
		m_renderer = std::make_shared<spr_renderer>();
		m_renderer->set_render_target(m_target);
		m_renderer->clear({ 0.4f, 0.4f, 0.4f });

		m_view_pos.setValue(x, y);
		m_view_size.setValue(int(w), int(h));

		// do not response user input
		EnableWindow(target_wnd, FALSE);
		ShowWindow(target_wnd, SW_NORMAL);

		debug("Sparrow view at (%d, %d, %d, %d)", x, y, x + w, y + h);

		return true;
	}

	void view_sparrow::on_render()
	{
		auto thread_id = std::this_thread::get_id();
		debug("start render on thread[0x%x], sparrow view", thread_id);

		D2D1_RECT_U src_rect = {
			0, 0, unsigned(m_view_size.x), unsigned(m_view_size.y)
		};
		D2D1_RECT_F dst_rect = {
			0.0f, 0.0f, float(m_view_size.x), float(m_view_size.y)
		};
		xsurface &color_buffer = m_target->get_color_buffer();
		size_t pitch = color_buffer.pitch();
		HRESULT result;
		result = m_bitmap->CopyFromMemory(&src_rect, color_buffer.get_buffer(), pitch);
		bool refresh = true;
		while (!application::get_instance()->is_exit())
		{
			m_d2d_rt->BeginDraw();
			if (refresh)
			{
				m_d2d_rt->DrawBitmap(m_bitmap, &dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
				refresh = false;
			}
			result = m_d2d_rt->EndDraw();
			if (result == D2DERR_RECREATE_TARGET)
			{
				warn("Render target lost!");
				rebuild_resource();
				refresh = true;
			}
			else if (result != S_OK)
			{
				warn("D2D end draw error!");
			}
			std::this_thread::sleep_for(std::chrono::microseconds(30));
		}

		debug("exit thread[0x%x]", thread_id);
	}

	void view_sparrow::set_text(const wchar_t * text)
	{
	}

	void view_sparrow::get_position(int & x, int & y)
	{
		x = m_view_pos.x; 
		y = m_view_pos.y;
	}

	void view_sparrow::get_size(unsigned & width, unsigned & height)
	{
		width = unsigned(m_view_size.x);
		height = unsigned(m_view_size.y);
	}

	bool view_sparrow::rebuild_resource()
	{
		if (!m_d2d_factory)
			return false;
		discard_resource();

		// create render target
		ID2D1HwndRenderTarget *ptr_render_target = nullptr;
		D2D1_PIXEL_FORMAT pixel_fmt = {
			DXGI_FORMAT_B8G8R8A8_UNORM,  // hardware or software
										 //DXGI_FORMAT_R8G8B8A8_UNORM,  // hardware only
			D2D1_ALPHA_MODE_IGNORE
		};
		D2D1_RENDER_TARGET_PROPERTIES render_property = {
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			pixel_fmt,
			0, 0, // dpiX and dpiY
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		};
		RECT client_rect;
		GetClientRect(m_hwnd, &client_rect);
		unsigned w = client_rect.right - client_rect.left;
		unsigned h = client_rect.bottom - client_rect.top;
		D2D1_HWND_RENDER_TARGET_PROPERTIES window_property = {
			m_hwnd,
			D2D1::SizeU(w, h),
			D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS,
		};
		HRESULT result = m_d2d_factory->CreateHwndRenderTarget(render_property, window_property, &ptr_render_target);
		if (result != S_OK)
		{
			error("Failed to create Direct2D render target.");
			return false;
		}

		// create bitmap
		D2D1_PIXEL_FORMAT bitmap_fmt = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D2D1_ALPHA_MODE_IGNORE
		};
		result = ptr_render_target->CreateBitmap({ w, h }, 0, 0, { bitmap_fmt , 0, 0 }, &m_bitmap);
		if (result != S_OK)
		{
			error("Failed to create D2D bitmap.");
			return false;
		}

		m_d2d_rt = ptr_render_target;
		return true;
	}

	void view_sparrow::discard_resource()
	{
		SAFE_RELEASE(m_d2d_rt);
		SAFE_RELEASE(m_bitmap);
	}

} // namespace wyc