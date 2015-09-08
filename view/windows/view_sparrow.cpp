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
		SAFE_RELEASE(m_d2d_rt);
		SAFE_RELEASE(m_d2d_factory);
		SAFE_RELEASE(m_bitmap);
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
		ID2D1HwndRenderTarget *ptr_render_target = nullptr;
		D2D1_PIXEL_FORMAT pixel_fmt = {
			//DXGI_FORMAT_B8G8R8A8_UNORM,  // hardware or software
			DXGI_FORMAT_R8G8B8A8_UNORM,  // hardware only
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
		GetClientRect(target_wnd, &client_rect);
		D2D1_HWND_RENDER_TARGET_PROPERTIES window_property = {
			target_wnd,
			D2D1::SizeU(client_rect.right - client_rect.left, client_rect.bottom - client_rect.top),
			D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS,
		};
		result = ptr_factory->CreateHwndRenderTarget(render_property, window_property, &ptr_render_target);
		if (result != S_OK)
		{
			error("Failed to create Direct2D render target.");
			return false;
		}

		m_hwnd = target_wnd;
		m_d2d_factory = ptr_factory;
		m_d2d_rt = ptr_render_target;

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

		m_target = std::make_shared<spr_render_target>();
		if (!m_target->create(w, h, SPR_COLOR_B8G8R8A8 | SPR_DEPTH_16))
		{
			m_target = nullptr;
			error("Failed to create sparrow render target.");
			return false;
		}
		m_renderer = std::make_shared<spr_renderer>();
		m_renderer->set_render_target(m_target);
		m_renderer->clear({ 0.0f, 1.0f, 0.0f });

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

		auto bitmap_size = m_bitmap->GetSize();
		xsurface &color_buffer = m_target->get_color_buffer();
		D2D1_RECT_U src_rect = {
			// left, top, right, bottom
			0, 0, color_buffer.row_length(), color_buffer.row()
		};
		D2D1_RECT_F dst_rect = {
			0.0f, 0.0f, 100, 100 // bitmap_size.width, bitmap_size.height
		};
		size_t pitch = color_buffer.pitch();
		HRESULT result;
		result = m_bitmap->CopyFromMemory(&src_rect, color_buffer.get_buffer(), pitch);
		while (!application::get_instance()->is_exit())
		{
			m_d2d_rt->BeginDraw();
			m_d2d_rt->Clear(D2D1::ColorF(D2D1::ColorF::White));
			m_d2d_rt->DrawBitmap(m_bitmap, &dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			result = m_d2d_rt->EndDraw();
			if (result != S_OK)
				break;
			std::this_thread::sleep_for(std::chrono::microseconds(30));
		}

		debug("exit thread[0x%x]", thread_id);
	}
} // namespace wyc