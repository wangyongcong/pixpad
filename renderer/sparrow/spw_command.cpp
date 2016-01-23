#include "spw_command.h"
#include <OpenEXR/ImathColorAlgo.h>
#include "spw_renderer.h"
#include "spw_rasterizer.h"
#include "vertex_layout.h"
#include <mathex/vecmath.h>

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
		CSurface &surf = renderer->m_rt->get_color_buffer();
		int surfh = surf.row(), surfw = surf.row_length();
		int halfw = surfw >> 1, halfh = surfh >> 1;
		CShaderFlatColor shader;
		set_orthograph(shader.m_uniform.mvp, -halfw, -halfh, 0.1f, halfw, halfh, 100.0f);
		auto &pipeline = renderer->m_pipeline;
		pipeline.setup(renderer->m_rt);
		pipeline.feed(mesh, &shader);
	}


#define GET_HANDLER(cmd_type) &spw_handler<cmd_type>

	spw_command_handler spw_cmd_map[CMD_COUNT] = {
		GET_HANDLER(cmd_test),
		GET_HANDLER(cmd_present),
		GET_HANDLER(cmd_clear),
		GET_HANDLER(cmd_draw_mesh),
	};

} // namespace wyc