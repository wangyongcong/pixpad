#include "spw_command.h"
#include "OpenEXR/ImathColorAlgo.h"
#include "spw_renderer.h"
#include "spw_rasterizer.h"

namespace wyc
{

#define SPW_CMD_HANDLER(cmd_type) template<> inline void spw_handler<cmd_type>(CSpwRenderer *renderer, RenderCommand *_cmd)
#define GET_HANDLER(cmd_type) &spw_handler<cmd_type>
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

	SPW_CMD_HANDLER(cmd_draw_mesh)
	{
		assert(renderer);
		auto *cmd = get_cmd(cmd_draw_mesh);
		assert(cmd);
		const CMesh *mesh = cmd->mesh;
		if (!mesh)
		{
			return;
		}
		CSurface& surf = renderer->m_rt->get_color_buffer();
		const int surf_w = surf.row_length(), surf_h = surf.row();
		int halfw = surf_w / 2, halfh = surf_h / 2;
		Imath::Box<Vec2i> scissor(Vec2i(-halfw, -halfh), Vec2i(halfw, halfh));
		Vec2i translate(halfw, surf_h - halfh - 1);
		auto plotter = [&surf, surf_w, surf_h, translate](int x, int y, float w0, float w1, float w2)
		{
			Vec3f c = { w0, w1, w2 };
			unsigned v = Imath::rgb2packed(c);
			x += translate.x;
			y = translate.y - y;
			assert(x >= 0 && x < surf_w);
			assert(y >= 0 && y < surf_h);
			surf.set(x, y, v);
		};
		auto &vb = mesh->vertex_buffer();
		assert(vb.vertex_size() == sizeof(VertexP3C3));
		Vec2f out[3];
		for (auto iter = vb.begin(), end = vb.end(); iter != end;)
		{
			const VertexP3C3 &v0 = *iter++;
			const VertexP3C3 &v1 = *iter++;
			const VertexP3C3 &v2 = *iter++;
			out[0].setValue(v0.pos.x, v0.pos.y);
			out[1].setValue(v1.pos.x, v1.pos.y);
			out[2].setValue(v2.pos.x, v2.pos.y);
			fill_triangle(scissor, out, plotter);
		}
	}


#define GET_HANDLER(cmd_type) &spw_handler<cmd_type>

	spw_command_handler spw_cmd_map[CMD_COUNT] = {
		GET_HANDLER(cmd_test),
		GET_HANDLER(cmd_present),
		GET_HANDLER(cmd_clear),
		GET_HANDLER(cmd_draw_mesh),
	};

} // namespace wyc