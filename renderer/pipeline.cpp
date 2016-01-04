#include "pipeline.h"
#include <cassert>
#include <OpenEXR/IlmThreadPool.h>
#include "mathex/vecmath.h"
#include "mathex/floatmath.h"
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
		constexpr int max_verts = 10;
		VertexOut cache[max_verts * 2];
		VertexOut *triangle = cache;
		VertexOut *verts_out = cache + max_verts;
		unsigned char i = 0;
		for (auto it = it_beg + beg; it != it_end; ++it)
		{
			const VertexIn &v = *it;
			vertex_shader(m_uniform, v, triangle[i++]);
			if (i == 3)
			{
				i = 0;
				// clipping
				size_t vcnt = 3;
				VertexOut *ret = clip_polygon(triangle, verts_out, vcnt, max_verts);
				if (!ret)
					continue;
				// viewport transform
				viewport_transform(m_uniform.viewport_center, m_uniform.viewport_radius, ret, vcnt);
				// send to fragment shader
			}
		}
	}

	void CPipeline::vertex_shader(const Uniform &uniform, const VertexIn & in, VertexOut & out)
	{
		Imath::V4f pos(in.pos);
		pos *= uniform.mvp;
		out.pos = pos;
		out.color = in.color;
	}

	void CPipeline::fragment_shader(const Uniform & uniform, const VertexOut & in, Fragment & out)
	{
	}

	template<>
	CPipeline::VertexOut intersect<CPipeline::VertexOut>(const CPipeline::VertexOut &p1, float d1, const CPipeline::VertexOut &p2, float d2)
	{
		float t = d1 / (d1 - d2);
		if (d1 < 0)
			t = fast_ceil(t * 1000) * 0.001f;
		else
			t = fast_floor(t * 1000) * 0.001f;
		const float *f1 = reinterpret_cast<const float*>(&p1);
		const float *f2 = reinterpret_cast<const float*>(&p2);
		CPipeline::VertexOut out;
		float *f3 = reinterpret_cast<float*>(&out);
		for (int i = 0; i < 7; ++i)
		{
			f3[i] = f1[i] * (1 - t) + f2[i] * t;
		}
		return out;
	}

	CPipeline::VertexOut* CPipeline::clip_polygon(VertexOut *in, VertexOut *out, size_t &size, size_t max_size)
	{
		float pdot, dot;
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		size_t prev_idx = size - 1;
		pdot = in[prev_idx].pos.w - w_epsilon;
		size_t out_size = 0;
		for (size_t i = 0; i < size; ++i)
		{
			const auto &pos = in[i].pos;
			dot = pos.w - w_epsilon;
			if (pdot * dot < 0) {
				assert(out_size < max_size && "vertex cache overflow");
				out[out_size++] = intersect(in[prev_idx], pdot, in[i], dot);
			}
			if (dot >= 0) {
				assert(out_size < max_size && "vertex cache overflow");
				out[out_size++] = in[i];
			}
			prev_idx = i;
			pdot = dot;
		}
		if (!out_size)
			return nullptr;
		std::swap(in, out);
		size = out_size;
		out_size = 0;
		// clipped by positive plane: W=X, W=Y, W=Z
		for (size_t i = 0; i < 3; ++i)
		{
			prev_idx = size - 1;
			const auto &pos = in[prev_idx].pos;
			float pdot = pos.w - pos[i], dot;
			for (size_t k = 0; k < size; ++k)
			{
				const auto &pos = in[k].pos;
				dot = pos.w - pos[i];
				if (pdot * dot < 0) {
					assert(out_size < max_size && "vertex cache overflow");
					out[out_size++] = intersect(in[prev_idx], pdot, in[k], dot);
				}
				if (dot >= 0) {
					assert(out_size < max_size && "vertex cache overflow");
					out[out_size++] = in[k];
				}
				prev_idx = i;
				pdot = dot;
			}
			if (!out_size)
				return nullptr;
			std::swap(in, out);
			size = out_size;
			out_size = 0;
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (size_t i = 0; i < 3; ++i)
		{
			prev_idx = size - 1;
			const auto &pos = in[prev_idx].pos;
			float pdot = pos.w + pos[i], dot;
			for (size_t k = 0; k < size; ++k)
			{
				const auto &pos = in[k].pos;
				dot = pos.w + pos[i];
				if (pdot * dot < 0)
				{
					assert(out_size < max_size && "vertex cache overflow");
					out[out_size++] = intersect(in[prev_idx], pdot, in[k], dot);
				}
				if (dot >= 0)
				{
					assert(out_size < max_size && "vertex cache overflow");
					out[out_size++] = in[k];
				}
				prev_idx = i;
				pdot = dot;
			}
			if (!out_size)
				return nullptr;
			std::swap(in, out);
			size = out_size;
			out_size = 0;
		}
		return in;
	}

	void CPipeline::viewport_transform(const Imath::V2f &center, const Imath::V2f &radius, VertexOut * in, size_t size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			auto &pos = in[i].pos;
			pos /= pos.w;
			pos.x = center.x + radius.x * pos.x;
			pos.y = center.y + radius.y * pos.y;
		}
	}

} // namesace wyc
