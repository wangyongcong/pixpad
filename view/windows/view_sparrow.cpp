#include "stdafx.h"
#include "view_sparrow.h"

#include <thread>

#include "log.h"
#include "glutil.h"
#include "windows_application.h"

namespace wyc
{
	bool view_sparrow::initialize(int x, int y, unsigned w, unsigned h)
	{
		windows_application* app_inst = dynamic_cast<windows_application*>(application::get_instance());
		if (!app_inst)
		{
			return false;
		}

		HINSTANCE os_instance = app_inst->os_instance();
		HWND main_wnd = app_inst->os_window();

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
		D2D1_RENDER_TARGET_PROPERTIES render_property = {
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			0, 0, // dpiX and dpiY
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		};
		D2D1_HWND_RENDER_TARGET_PROPERTIES window_property;
		result = ptr_factory->CreateHwndRenderTarget(render_property, window_property, &ptr_render_target);
		if (result != S_OK)
		{
			error("Failed to create Direct2D render target.");
			return false;
		}

		return true;
	}

	void view_sparrow::on_render()
	{
		auto thread_id = std::this_thread::get_id();
		debug("start render on thread[0x%x], sparrow view", thread_id);


		while (!application::get_instance()->is_exit())
		{
			std::this_thread::sleep_for(std::chrono::microseconds(30));
		}

		debug("exit thread[0x%x]", thread_id);
	}
} // namespace wyc