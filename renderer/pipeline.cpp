#include "pipeline.h"
#include <OpenEXR/IlmThreadPool.h>
#include "mathex/vecmath.h"
#include "thread/platform_info.h"

namespace wyc
{
	CPipeline::CPipeline()
		: m_num_core(0)
	{
		if (0 == m_num_core)
		{
			// use as many cores as possible
			m_num_core = core_num();
		}
	}


	void CPipeline::feed(const CMesh &mesh)
	{
		const CVertexBuffer &vb = mesh.vertex_buffer();
		size_t vert_cnt = vb.size();
		size_t tri_cnt = vert_cnt / 3;
		//size_t tri_per_core = tri_cnt / m_num_core;
		
		// todo: work parallel
		stage_vertex(vb, 0, vert_cnt);
	}

	void CPipeline::stage_vertex(const CVertexBuffer &vb, size_t beg, size_t end)
	{
		auto it_beg = vb.begin();
		auto it_end = it_beg + end;
		constexpr int prim_verts = 3;
		std::vector<VertexOut> triangle;
		triangle.resize(prim_verts);
		// clipped by 6 planes may result 6 more vertices at most
		std::vector<VertexOut> verts_out;
		verts_out.reserve(prim_verts + 3);
		unsigned char i = 0;
		for (auto it = it_beg + beg; it != it_end; ++it)
		{
			const VertexIn &v = *it;
			vertex_shader(v, triangle[i++]);
			if (i == 3)
			{
				i = 0;
				clip(triangle, verts_out);
				// send to fragment shader
			}
		}
	}

	void CPipeline::vertex_shader(const VertexIn & in, VertexOut & out)
	{
		Imath::V4f pos(in.pos);
		pos *= m_mvp;
		out.pos = pos;
		out.color = in.color;
	}

	void CPipeline::clip(const std::vector<VertexOut>& vertices, std::vector<VertexOut>& out)
	{
		size_t vcnt = vertices.size();
		float pdot, dot;
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		size_t prev_idx = vcnt - 1;
		pdot = vertices.back().pos.w - w_epsilon;
		for (size_t i = 0; i < vcnt; ++i)
			//for (auto &vert : vertices)
		{
			const VertexOut &vert = vertices[i];
			const VertexOut &prev = vertices[prev_idx];
			dot = vert.pos.w - w_epsilon;
			if (pdot * dot < 0)
				out.push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out.push_back(vert);
			//prev = vert;
			prev_idx = i;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by positive plane: W=X, W=Y, W=Z
		for (int i = 0; i < 3; ++i)
		{
			_clip_comp(&vertices, &out, i);
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			_clip_comp_neg(&vertices, &out, i);
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
	}


} // namesace wyc
