#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include "mathex/vecmath.h"
#include "mathex/floatmath.h"
#include "thread/platform_info.h"
#include "spw_rasterizer.h"

namespace wyc
{
	CSpwPipeline::CSpwPipeline()
		: m_num_core(0)
		, m_clock_wise(COUNTER_CLOCK_WISE)
	{
		if (0 == m_num_core)
		{
			// use as many cores as possible
			m_num_core = core_num();
		}
	}

	CSpwPipeline::~CSpwPipeline()
	{
		m_rt = nullptr;
	}

	void CSpwPipeline::setup(std::shared_ptr<CSpwRenderTarget> rt)
	{
		m_rt = rt;
		CSurface &surf = rt->get_color_buffer();
		int surfh = surf.row(), surfw = surf.row_length();
		int halfw = surfw >> 1, halfh = surfh >> 1;
		m_region.center = { halfw, halfh };
		m_region.center_device = { halfw, surfh - halfh - 1 };
		m_region.block.min = { -halfw, -halfh };
		m_region.block.max = {  halfw,  halfh };
		m_region.block_device.min = { 0, 0 };
		m_region.block_device.max = { surfw, surfh };
		set_viewport({ {0, 0}, {surfw, surfh} });
		set_orthograph(m_uniform.mvp, -halfw, -halfh, 0.1f, halfw, halfh, 100.0f);
	}

	void CSpwPipeline::feed(const CMesh &mesh)
	{
		const CVertexBuffer &vb = mesh.vertex_buffer();
		size_t vert_cnt = vb.size();
		size_t tri_cnt = vert_cnt / 3;
		//size_t tri_per_core = tri_cnt / m_num_core;
		
		// todo: work parallel
		process(vb, 0, vert_cnt);
	}

	void CSpwPipeline::process(const CVertexBuffer &vb, size_t beg, size_t end)
	{
		constexpr size_t comp_per_vertex = VertexIn::component;
		constexpr size_t idx_pos = VertexIn::index_pos;
		// use triangle as the basic primitive (3 vertex)
		// clipping may produce 7 more vertex
		// so the maximum vertex count is 10
		constexpr size_t cache_size = comp_per_vertex * 10;
		float prim_cache[cache_size];
		float clip_cache[cache_size];
		float* const prim_end = prim_cache + comp_per_vertex * 3;

		auto stream_it = vb.stream_begin();
		auto stream_end = vb.stream_end();
		float *prim = prim_cache, *clip_out = clip_cache;
		for (auto it = stream_it + beg; it != stream_end; ++it)
		{
			const float *vert = *stream_it;
			vertex_shader(m_uniform, *(const VertexIn*)vert, *(VertexOut*)prim);
			prim += comp_per_vertex;
			if (prim != prim_end)
				continue;
			// emit primitive
			prim = prim_cache;
			// clipping
		}

		//constexpr int max_verts = 10;
		//VertexOut cache[max_verts * 2];
		//VertexOut *triangle = cache;
		//VertexOut *verts_out = cache + max_verts;
		//unsigned char i = 0;
		//auto it_beg = vb.begin();
		//auto it_end = it_beg + end;
		//for (auto it = it_beg + beg; it != it_end; ++it)
		//{
		//	const VertexIn &v = *it;
		//	vertex_shader(m_uniform, v, triangle[i++]);
		//	if (i == 3)
		//	{
		//		i = 0;
		//		// clipping
		//		size_t vcnt = 3;
		//		VertexOut *ret = clip_polygon(triangle, verts_out, vcnt, max_verts);
		//		if (!ret)
		//			continue;
		//		// viewport transform
		//		viewport_transform(m_uniform.viewport_center, m_uniform.viewport_radius, ret, vcnt);
		//		// draw triangles
		//		for (size_t j = 2; j < vcnt; ++j)
		//		{
		//			draw_triangle(ret[0], ret[j - 1], ret[j]);
		//		}
		//	}
		//}
	}

	void CSpwPipeline::vertex_shader(const Uniform & uniform, const VertexIn & in, VertexOut & out)
	{
		Imath::V4f pos(in.pos);
		pos.z = -1.0f;
		pos = uniform.mvp * pos;
		out.pos = pos;
		out.color = in.color;
	}

	void CSpwPipeline::fragment_shader(const Uniform & uniform, const VertexOut & in, Fragment & out)
	{
		out.color = in.color;
	}

	template<>
	CSpwPipeline::VertexOut intersect<CSpwPipeline::VertexOut>(const CSpwPipeline::VertexOut &p1, float d1, const CSpwPipeline::VertexOut &p2, float d2)
	{
		float t = d1 / (d1 - d2);
		if (d1 < 0)
			t = fast_ceil(t * 1000) * 0.001f;
		else
			t = fast_floor(t * 1000) * 0.001f;
		const float *f1 = reinterpret_cast<const float*>(&p1);
		const float *f2 = reinterpret_cast<const float*>(&p2);
		CSpwPipeline::VertexOut out;
		float *f3 = reinterpret_cast<float*>(&out);
		for (int i = 0; i < 7; ++i)
		{
			f3[i] = f1[i] * (1 - t) + f2[i] * t;
		}
		return out;
	}

	template<>
	inline CSpwPipeline::VertexOut interpolate<CSpwPipeline::VertexOut>(
		const CSpwPipeline::VertexOut &v0, const CSpwPipeline::VertexOut &v1, const CSpwPipeline::VertexOut &v2, 
		float t0, float t1, float t2)
	{
		CSpwPipeline::VertexOut out;
		out.pos = v0.pos * t0 + v1.pos * t1 + v2.pos * t2;
		out.color = v0.color * t0 + v1.color * t1 + v2.color * t2;
		return out;
	}

	CSpwPipeline::VertexOut* CSpwPipeline::clip_polygon(VertexOut *in, VertexOut *out, size_t &size, size_t max_size)
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

	void CSpwPipeline::viewport_transform(const Imath::V2f &center, const Imath::V2f &radius, VertexOut * in, size_t size)
	{
		for (size_t i = 0; i < size; ++i)
		{
			auto &pos = in[i].pos;
			// after homogenized, (x, y) are within [-1, 1]
			pos /= pos.w;
			pos.x = center.x + radius.x * pos.x;
			pos.y = center.y + radius.y * pos.y;
		}
	}

	void CSpwPipeline::draw_triangle(const VertexOut & v0, const VertexOut & v1, const VertexOut & v2)
	{
		// backface culling
		Imath::V2f v01(v0.pos.x - v1.pos.x, v0.pos.y - v1.pos.y);
		Imath::V2f v21(v2.pos.x - v1.pos.x, v2.pos.y - v1.pos.y);
		if (v01.cross(v21) * m_clock_wise <= 0)
		{
			return;
		}
		// send to rasterizer
		using namespace std::placeholders;
		auto plotter = std::bind(&CSpwPipeline::write_fragment, this, _1, _2, _3);
		Imath::V2f p0 = { v0.pos.x - m_region.center.x, v0.pos.y - m_region.center.y };
		Imath::V2f p1 = { v1.pos.x - m_region.center.x, v1.pos.y - m_region.center.y };
		Imath::V2f p2 = { v2.pos.x - m_region.center.x, v2.pos.y - m_region.center.y };
		fill_triangle(m_region.block, p0, p1, p2, v0, v1, v2, plotter);
	}

	void CSpwPipeline::write_fragment(int x, int y, VertexOut & in)
	{
		Fragment out;
		fragment_shader(m_uniform, in, out);
		// write fragment buffer
		auto &surf = m_rt->get_color_buffer();
		unsigned v = Imath::rgb2packed(out.color);
		x += m_region.center_device.x;
		y = m_region.center_device.y - y;
		assert(inside(x, y, m_region.block_device));
		surf.set(x, y, v);
	}

} // namesace wyc
