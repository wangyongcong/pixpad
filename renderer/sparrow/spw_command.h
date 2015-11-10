#pragma once

#include "OpenEXR/ImathColorAlgo.h"
#include "render_command.h"
#include "spw_renderer.h"
#include "spw_rasterizer.h"

namespace wyc
{

#define SPW_CMD_HANDLER(cmd_type) template<> inline void spw_handler<cmd_type>(CSpwRenderer *renderer, RenderCommand *_cmd)
	
#define get_cmd(cmd_type) static_cast<cmd_type*>(_cmd)

	SPW_CMD_HANDLER(cmd_test)
	{
		auto cmd = get_cmd(cmd_test);
		for (auto &v : *cmd->jobs)
		{
			v = 1;
		}
	}

	SPW_CMD_HANDLER(cmd_present)
	{
		renderer->spw_present();
		auto *cmd = get_cmd(cmd_present);
		cmd->is_done.set_value();
	}

	SPW_CMD_HANDLER(cmd_clear)
	{
		assert(renderer);
		auto *cmd = get_cmd(cmd_clear);
		assert(cmd);
		if (!renderer->m_rt)
		{
			return;
		}
		CSurface& surf = renderer->m_rt->get_color_buffer();
		uint32_t v = Imath::rgb2packed(cmd->color);
		surf.clear(v);
	}

	SPW_CMD_HANDLER(cmd_test_triangle)
	{
		assert(renderer);
		auto *cmd = get_cmd(cmd_test_triangle);
		assert(cmd);
		if (!renderer->m_rt)
		{
			return;
		}
		float radius = cmd->radius;
		if (radius <= 0)
		{ 
			return;
		}
		CSurface& surf = renderer->m_rt->get_color_buffer();
		const int surf_w = surf.row_length(), surf_h = surf.row();
		int offx = surf_w / 2, offy = surf_h / 2;
		Imath::Box<Vec2i> scissor(Vec2i(-offx, -offy), Vec2i(offx, offy));
		const float sin30 = 0.5f, cos30 = 0.866f;
		Vec2f verts[3] = {
			{ 0, radius },
			{ -radius * cos30, -radius * sin30 },
			{  radius * cos30, -radius * sin30 },
		};
		offy = surf_h - offy - 1;
		auto plotter = [&surf, surf_w, surf_h, offx, offy](int x, int y, float w0, float w1, float w2)
		{
			//const Vec3f colors[3] = {
			//	{ 1.0f, 0.0f, 0.0f },
			//	{ 0.0f, 1.0f, 0.0f },
			//	{ 0.0f, 0.0f, 1.0f },
			//};
			//Vec3f c = colors[0] * w0 + colors[1] * w1 + colors[2] * w2;
			Vec3f c = { w0, w1, w2 };
			unsigned v = Imath::rgb2packed(c);
			x += offx;
			y = offy - y;
			assert(x >= 0 && x < surf_w);
			assert(y >= 0 && y < surf_h);
			surf.set(x, y, v);
		};
		fill_triangle(scissor, verts, plotter);
	}


} // namespace wyc