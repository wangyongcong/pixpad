#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>
#include "mathex/vecmath.h"
#include "thread/platform_info.h"
#include "spw_rasterizer.h"
#include "clipping.h"

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

	void viewport_transform(const Imath::V2f &center, const Imath::V2f &radius, float* vertex_pos, size_t size, size_t stride)
	{
		for (size_t i = 0; i < size; ++i, vertex_pos += stride)
		{
			// after homogenized, (x, y) are within [-1, 1]
			Imath::V4f &pos = *(Imath::V4f*)vertex_pos;
			pos /= pos.w;
			pos.x = center.x + radius.x * pos.x;
			pos.y = center.y + radius.y * pos.y;
		}
	}

	class CSpwPlotter
	{
	public:
		const float *v0, *v1, *v2;
		CSpwPlotter(const CSpwPipeline *pipeline, size_t stride)
			: m_pipeline(pipeline)
			, m_stride(stride)
		{
		}

		void operator() (float w1, float w2, float w3)
		{

		}

	private:
		const CSpwPipeline *m_pipeline;
		size_t m_stride;
	};

	void CSpwPipeline::draw_triangles(const float *vertices, size_t count, size_t stride, size_t pos_offset) const
	{
		CSpwPlotter plotter(this, stride);
		const float *v0 = vertices;
		const float *v1 = v0 + stride;
		const float *v2 = v1 + stride;
		Imath::V2f tpos[3];
		for (size_t j = 2; j < count; ++j)
		{
			const Imath::V2f &p0 = *(Imath::V2f*)(v0 + pos_offset);
			const Imath::V2f &p1 = *(Imath::V2f*)(v1 + pos_offset);
			const Imath::V2f &p2 = *(Imath::V2f*)(v2 + pos_offset);
			// backface culling
			Imath::V2f v10(p0.x - p1.x, p0.y - p1.y);
			Imath::V2f v12(p2.x - p1.x, p2.y - p1.y);
			if (v10.cross(v12) * m_clock_wise <= 0)
				return;
			// send to rasterizer
			tpos[0] = p0 - m_region.center;
			tpos[1] = p1 - m_region.center;
			tpos[2] = p2 - m_region.center;
			// todo: calculate triangle bounding box, and intersect with region block
			//fill_triangle(m_region.block, tpos[0], tpos[1], tpos[2], plotter);
			// next one
			v0 = v1;
			v1 = v2;
			v2 += stride;
		}
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

	void CSpwPipeline::process(const CVertexBuffer &vb, size_t beg, size_t end) const
	{
		constexpr size_t stride = VertexIn::component;
		constexpr size_t pos_offset = VertexIn::index_pos;
		// use triangle as the basic primitive (3 vertex)
		// clipping may produce 7 more vertex
		// so the maximum vertex count is 10
		constexpr size_t cache_size = stride * 10;
		float prim_cache[cache_size];
		float clip_cache[cache_size];

		float *prim = prim_cache;
		float* const prim_end = prim_cache + stride * 3;
		auto stream_it = vb.stream_begin();
		auto stream_end = vb.stream_end();
		for (auto it = stream_it + beg; it != stream_end; ++it)
		{
			const float *vert = *stream_it;
			vertex_shader(m_uniform, *(const VertexIn*)vert, *(VertexOut*)prim);
			prim += stride;
			if (prim != prim_end)
				continue;
			// emit primitive
			prim = prim_cache;
			// clipping
			size_t vcnt = 3;
			float *out = clip_polygon_stream(prim_cache, clip_cache, vcnt, stride, pos_offset, cache_size);
			if (!out)
				continue;
			// viewport transform
			viewport_transform(m_uniform.viewport_center, m_uniform.viewport_radius, out + pos_offset, vcnt, stride);
			// draw triangles
			draw_triangles(out, vcnt, stride, pos_offset);
		}
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

/*	template<>
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
*/
} // namesace wyc
