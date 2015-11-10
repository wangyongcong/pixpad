#include "stdafx.h"
#include "game_pixpad.h"

#include <ctime>
#include <memory>
#include <functional>
#include <thread>

#include <OpenEXR/ImathFun.h>
#include <OpenEXR/ImathColor.h>

#include "application.h"
#include "game_config.h"
#include "log.h"
#include "util.h"

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
		for (auto &ptr : m_renderers)
		{
			ptr->get_ready();
		}
		debug("all renderers are ready.");

		// start a task

		return;
	}

	void CGamePixpad::on_close()
	{
		m_signal_exit.store(true, std::memory_order_relaxed);
		for (auto &pthread : m_thread_pool)
		{
			pthread.join();
		}
	}

	void CGamePixpad::on_update()
	{
		auto t_start = std::chrono::high_resolution_clock::now();
		for (auto &ptr : m_renderers)
		{
			auto *clear = ptr->new_command<cmd_clear>();
			//clear->color.setValue(0.4f, 0.4f, 0.4f);
			clear->color.setValue(0.0f, 0.0f, 0.0f);
			ptr->enqueue(clear);
			auto *test = ptr->new_command<cmd_test_triangle>();
			test->radius = 100.0f;
			ptr->enqueue(test);

			ptr->present();
		}
		// wait for frame
		for (auto &ptr : m_renderers)
		{
			ptr->end_frame();
		}
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
				m_renderers.push_back(ptr_view->get_renderer());
				auto func = std::bind(&CViewBase::on_render, ptr_view);
				if (ptr_view) 
				{
					m_thread_pool.push_back(std::thread(func));
				}
				x += view_w;
			}
			x = 0;
			y += view_h;
		}
	}

} // namespace wyc

  //void CGamePixpad::on_paint()
  //{
  //	m_redraw = true;
  //	typedef Imath::C3c color_t;
  //	size_t vw, vh;
  //	application::get_instance()->get_window_size(vw, vh);
  //	color_t c = { 0, 0, 0 };
  //	m_surf.storage(vw, vh, sizeof(c));
  //	m_surf.clear(c);

  //	size_t lx, ly, rx, ry;
  //	lx = vw >> 2;
  //	rx = vw - lx;
  //	ly = vh >> 2;
  //	ry = vh - ly;

  //	// draw viewport frame
  //	c = { 255, 255, 0 };
  //	m_surf.set_line(ly, c, lx, rx);
  //	m_surf.set_line(ry - 1, c, lx, rx);
  //	for (size_t i = ly; i < ry; ++i)
  //	{
  //		m_surf.set(lx, i, c);
  //		m_surf.set(rx - 1, i, c);
  //	}
  //	draw_cube(lx, ly, rx, ry);
  //}

  //void CGamePixpad::random_triangle(int lx, int ly, int rx, int ry)
  //{
  //	std::vector<vec4f> planes;
  //	// left plane
  //	planes.push_back({ 1, 0, 0, 1 });
  //	// top plane
  //	planes.push_back({ 0, -1, 0, 1 });
  //	// right plane
  //	planes.push_back({ -1, 0, 0, 1 });
  //	// bottom plane
  //	planes.push_back({ 0, 1, 0, 1 });

  //	std::vector<vec3f> vertices;
  //	for (int i = 0; i < 3; ++i)
  //	{
  //		float x = (float)m_rnd.nextf();
  //		float y = (float)m_rnd.nextf();
  //		// clamp to [-2, 2]
  //		x = (x - 0.5f) * 4;
  //		y = (y - 0.5f) * 4;
  //		vertices.push_back({ x, y, 0 });
  //		info("v%d: (%f, %f)", i, x, y);
  //	}
  //	wyc::clip_polygon(planes, vertices);

  //	// transfrom to screen space
  //	float half_vw, half_vh;
  //	half_vw = (rx - lx - 0.5f) * 0.5f;
  //	half_vh = (ry - ly - 0.5f) * 0.5f;
  //	for (auto &v : vertices)
  //	{
  //		float tmp;
  //		tmp = Imath::clamp(v.x, -1.0f, 1.0f);
  //		v.x = lx + half_vw * (tmp + 1);
  //		tmp = Imath::clamp(v.y, -1.0f, 1.0f);
  //		v.y = ly + half_vh * (tmp + 1);
  //		assert(v.x >= lx && v.x < rx);
  //		assert(v.y >= ly && v.y < ry);
  //	}

  //	if (vertices.size()>2)
  //	{
  //		xplotter<Imath::C3c> plot(m_surf, Imath::C3c(0, 255, 0));
  //		auto v0 = vertices.back();
  //		for (auto &v1 : vertices)
  //		{
  //			draw_line(v0, v1, plot);
  //			v0 = v1;
  //		}
  //		v0 = vertices.front();
  //		for (size_t i = 2, end = vertices.size() - 1; i < end; ++i)
  //		{
  //			auto &v1 = vertices[i];
  //			draw_line(v0, v1, plot);
  //		}
  //	}
  //	else
  //	{
  //		warn("vertices left: %d", vertices.size());
  //	}
  //}

  //void CGamePixpad::draw_cube(int lx, int ly, int rx, int ry)
  //{
  //	std::vector<vec3f> vertices;
  //	std::vector<unsigned short> faces;
  //	wyc::box(0.4f, vertices, faces);

  //	mat4f proj, m1, m2;
  //	wyc::set_perspective(proj, 45, 4.f / 3, 1, 100);
  //	// rotate
  //	wyc::set_rotate_x(m1, wyc::deg2rad(45.f));
  //	wyc::set_rotate_y(m2, wyc::deg2rad(30.f));
  //	m1 *= m2;
  //	// tranlate
  //	//m1[0][3] = 0.f;
  //	//m1[1][3] = 0.f;
  //	m1[2][3] = -1.4f;
  //	// finally the MVP matrix
  //	proj *= m1;

  //	// viewing vector 
  //	vec3f camera_pos = { 0, 0, 0 };
  //	// translate to object space
  //	if (m2.inverse_of(m1)) 
  //		camera_pos = m2 * camera_pos;

  //	float half_vw, half_vh;
  //	half_vw = (rx - lx - 0.5f) * 0.5f;
  //	half_vh = (ry - ly - 0.5f) * 0.5f;

  //	std::vector<vec4f> verts_cache;
  //	verts_cache.reserve(9);
  //	vec3f v0, v1, v2, n, view;
  //	int beg = 0, end = faces.size();
  //	for (int i = beg + 2; i < end; i += 3)
  //	{
  //		// projection
  //		v0 = vertices[faces[i - 2]];
  //		v1 = vertices[faces[i - 1]];
  //		v2 = vertices[faces[i]];
  //		// backface culling
  //		n = v1 - v0;
  //		n = n.cross(v2 - v0);
  //		n.normalize();
  //		view = camera_pos - v0;
  //		view.normalize();
  //		if ((n ^ view) < 0)
  //			continue;
  //		// MVP transform
  //		verts_cache.push_back(proj * v0);
  //		verts_cache.push_back(proj * v1);
  //		verts_cache.push_back(proj * v2);
  //		// clipping
  //		wyc::clip_polygon_homo(verts_cache);
  //		if (verts_cache.empty())
  //			continue;
  //		// viewport transform
  //		for (auto &v : verts_cache)
  //		{
  //			v /= v.w;
  //			float tmp;
  //			tmp = Imath::clamp(v.x, -1.0f, 1.0f);
  //			v.x = lx + half_vw * (tmp + 1);
  //			tmp = Imath::clamp(v.y, -1.0f, 1.0f);
  //			v.y = ly + half_vh * (tmp + 1);
  //			assert(v.x >= lx && v.x < rx);
  //			assert(v.y >= ly && v.y < ry);
  //		}
  //		// draw
  //		draw_triangles(verts_cache);
  //		// clear cache
  //		verts_cache.clear();
  //	}

  //}

  //void CGamePixpad::draw_triangles(const std::vector<vec4f> &vertices)
  //{
  //	xplotter<Imath::C3c> plot(m_surf, Imath::C3c(0, 255, 0));
  //	auto v0 = vertices.back();
  //	for (auto &v1 : vertices)
  //	{
  //		draw_line(v0, v1, plot);
  //		v0 = v1;
  //	}
  //	v0 = vertices.front();
  //	for (size_t i = 2, end = vertices.size() - 1; i < end; ++i)
  //	{
  //		auto &v1 = vertices[i];
  //		draw_line(v0, v1, plot);
  //	}
  //}
