#include "stdafx.h"
#include "game_pixpad.h"

#include <ctime>
#include <memory>
#include <functional>
#include <thread>

#include <OpenEXR/ImathFun.h>
#include <OpenEXR/ImathColor.h>
#include <OpenEXR/ImathFrustum.h>

#include "application.h"
#include "game_config.h"
#include "log.h"
#include "util.h"
#include "mesh.h"
#include <sparrow/shader/tex_color.h>
#include <mathex/vecmath.h>
#include <common/image.h>

namespace wyc
{
	CGamePixpad::CGamePixpad() : 
		m_game_name(L"Game Pixpad"), 
		m_signal_exit(false),
		m_frame(0)
	{
	}

	void CGamePixpad::on_start()
	{
		unsigned core_count = std::thread::hardware_concurrency();
		debug("max thread count: %d", core_count);
		create_views();
		for (auto &ptr : m_views)
		{
			ptr->get_renderer()->wait_for_ready();
			ptr->suspend();
		}
		debug("all renderers are ready.");

		start_task();
	}

	void CGamePixpad::on_close()
	{
		m_signal_exit.store(true, std::memory_order_relaxed);
		for (auto &ptr : m_views)
		{
			ptr->wake_up();
		}
		for (auto &pthread : m_thread_pool)
		{
			pthread.join();
		}
	}

	void CGamePixpad::on_update()
	{
		auto t_start = std::chrono::high_resolution_clock::now();
		auto t_end = std::chrono::high_resolution_clock::now();
		auto frame_time = std::chrono::duration_cast<duration_t>(t_end - t_start);
		m_total_time += frame_time;
		m_frame_time[m_frame % m_frame_time.size()] = frame_time;
		m_frame += 1;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	bool CGamePixpad::is_exit()
	{
		return m_signal_exit.load(std::memory_order_consume);
	}

	void CGamePixpad::on_key_down(int keycode)
	{
		if (keycode == VK_ESCAPE) {
			::PostQuitMessage(0);
			return;
		}
	}

	void CGamePixpad::create_views()
	{
		unsigned view_count = next_power2(c_view_count);
		unsigned n = log2p2(view_count);
		unsigned row, col;
		row = 1 << (n / 2);
		if (0 == (n & 1))
		{
			col = row;
		}
		else
		{
			col = row << 1;
		}
		debug("create %d x %d views", row, col);
		CApplication *app_inst = CApplication::get_instance();
		size_t window_w, window_h;
		app_inst->get_window_size(window_w, window_h);
		unsigned view_w, view_h;
		view_w = window_w / col;
		view_h = window_h / row;
		unsigned client_w = view_w * col, client_h = view_h * row;
		if (client_w != window_w || client_h != window_h)
		{
			app_inst->resize(client_w, client_h);
		}

		static_assert(c_view_count <= sizeof(c_view_list) / sizeof(wyc::EViewType), "Invalid view count.");

		int x = 0, y = 0;
		unsigned view_idx = 0;
		for (unsigned r = 0; r < row && view_idx < c_view_count; ++r)
		{
			for (unsigned c = 0; c < col && view_idx < c_view_count; ++c, ++view_idx)
			{
				auto ptr_view = CViewBase::create_view(c_view_list[view_idx], x, y, view_w, view_h);
				if (ptr_view) 
				{
					m_views.push_back(ptr_view);
					auto func = std::bind(&CViewBase::on_render, ptr_view);
					m_thread_pool.push_back(std::thread(func));
				}
				x += view_w;
			}
			x = 0;
			y += view_h;
		}
	}

	void CGamePixpad::start_task()
	{
		for (auto &ptr : m_views)
		{
			ptr->wake_up();
		}
		// start a task
		//CTriangleMesh triangle(100);
		//CQuadMesh quad(100, 100);
		CMesh cube;
		if (!cube.load_obj(L"res/cube/cube.obj"))
			return;
		CImage image;
		if (!image.load(L"res/cube/default.png"))
			return;
		// material
		CShaderTexColor shader;
		set_orthograph(shader.m_uniform.mvp, -400, -300, 0.1f, 400, 300, 1000.0f);
		Imath::M44f transform;
		set_scale(transform, 100, 100, 100);
		transform[0][3] = 0;
		transform[1][3] = 0;
		transform[2][3] = -100;
		shader.m_uniform.mvp *= transform;
		shader.m_uniform.color = { 1.0f, 0.0f, 1.0f, 1.0f };
		//Imath::Frustumf frustum;
		//Imath::Matrix44<float> mvp = frustum.projectionMatrix();
		for (auto &ptr : m_views)
		{
			auto renderer = ptr->get_renderer();
			auto *clear = renderer->new_command<cmd_clear>();
			clear->color.setValue(0.0f, 0.0f, 0.0f);
			renderer->enqueue(clear);
			auto *draw = renderer->new_command<cmd_draw_mesh>();
			draw->mesh = &cube;
			draw->program = &shader;
			renderer->enqueue(draw);

			renderer->present();
		}
		// wait for frame
		for (auto &ptr : m_views)
		{
			ptr->get_renderer()->end_frame();
			ptr->suspend();
		}
	}

} // namespace wyc

