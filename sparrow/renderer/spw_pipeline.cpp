#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include "ImathBoxAlgoExt.h"
#include "vecmath.h"
#include "spw_rasterizer.h"
#include "clipping.h"

namespace wyc
{
	CSpwPipeline::CSpwPipeline()
		: m_num_core(1)
		, m_clock_wise(COUNTER_CLOCK_WISE)
		, m_draw_mode(FILL_MODE)
	{
	}

	CSpwPipeline::~CSpwPipeline()
	{
		m_rt = nullptr;
	}

	void CSpwPipeline::setup(std::shared_ptr<CSpwRenderTarget> rt)
	{
		m_rt = rt;
		unsigned surfw, surfh;
		rt->get_size(surfw, surfh);
		set_viewport({ {0, 0}, {int(surfw), int(surfh)} });
	}

	bool CSpwPipeline::check_material(const AttribDefine & attrib_def) const
	{
		if (!attrib_def.in_count || !attrib_def.out_count)
			return false;
		if (attrib_def.out_attribs[0].usage != ATTR_POSITION && attrib_def.out_attribs[0].component != 4)
			return false;
		return true;
	}

	void CSpwPipeline::feed(const CMesh *mesh, const CMaterial *material)
	{
		assert(mesh && material);
		const CVertexBuffer &vb = mesh->vertex_buffer();
		const CIndexBuffer &ib = mesh->index_buffer();
		if (ib.stride() != sizeof(unsigned)) {
			log_warn("Vertex index should be 32bit integer.");
			return;
		}
		// setup render target
		unsigned surfw, surfh;
		m_rt->get_size(surfw, surfh);
		int halfw = surfw >> 1, halfh = surfh >> 1;

		// bind stream
		auto &attrib_def = material->get_attrib_define();
		if (!check_material(attrib_def))
			return;
		std::vector<AttribStream> attrib_stream;
		attrib_stream.resize(attrib_def.in_count);
		for (unsigned i = 0; i < attrib_def.in_count; ++i)
		{
			auto &slot = attrib_def.in_attribs[i];
			if (!vb.has_attribute(slot.usage)
				|| vb.attrib_component(slot.usage) < slot.component)
				return;
			attrib_stream[i] = {
				(const char*)vb.attrib_stream(slot.usage),
				vb.attrib_stride(slot.usage),
			};
		}

		// todo: work parallel
		RasterTask task;
		task.material = material;
		task.index_stream = ib.get_index_stream();
		task.index_size = ib.size() / 3 * 3;
		task.index_stride = ib.stride();
		task.in_stream = &attrib_stream[0];
		task.in_count = attrib_stream.size();
		task.in_stride = attrib_def.in_stride;
		task.out_count = attrib_def.out_count;
		task.out_stride = attrib_def.out_stride;
		// assign surface block
		task.block = { { 0, 0 },{ halfw, halfh } };
		// use triangle as the basic primitive (3 vertex)
		// clipping may produce 7 more vertex
		// so the maximum vertex count is 10
		constexpr int max_count = 10;
		// cache for vertex attributes 
		// we use double buffer to swap input/output
		size_t cache_vert = sizeof(float) * attrib_def.out_stride * max_count * 2;
		// cache for clipping position
		size_t cache_frag = sizeof(float) * (task.out_stride - 4);
		task.cache_size = cache_vert + cache_frag;
		task.cache = new char[task.cache_size];
		task.vert_cache0 = reinterpret_cast<float*>(task.cache);
		task.vert_cache1 = task.vert_cache0 + attrib_def.out_stride * max_count;
		task.frag_cache = reinterpret_cast<float*>(task.cache + cache_vert);
		task.frag_stride = task.out_stride - 4;
		//task.clip_cache0 = reinterpret_cast<Vec4f*>(task.cache + cache_vert);
		//task.clip_cache1 = task.clip_cache0 + max_count;

		process(task);
	}

	void CSpwPipeline::set_viewport(const Imath::Box2i & view)
	{
		auto _tmp = view.max + view.min;
		m_vp_translate = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		_tmp = view.max - view.min;
		m_vp_scale = { _tmp.x * 0.5f, _tmp.y * 0.5f };
	}

	void CSpwPipeline::process(RasterTask &task) const
	{
		typedef unsigned index_t;
		const index_t *index_stream = reinterpret_cast<const index_t*>(task.index_stream);
		std::vector<const char*> attrib_ptr(task.in_count, nullptr);
		const void* in_vertex = &attrib_ptr[0];
		for (size_t i = 0; i < task.index_size; ++i)
		{
			index_t idx_vert = index_stream[i];
			for (size_t j = 0; j < task.in_count; ++j)
			{
				auto &stream = task.in_stream[j];
				attrib_ptr[j] = stream.first + stream.second * idx_vert;
			}
			size_t vcnt = i % 3;
			task.material->vertex_shader(in_vertex, task.vert_cache0 + task.out_stride * vcnt);
			if (vcnt == 2)
			{
				size_t clip_count = 3;
				float *clip_result = clip_polygon_stream(task.vert_cache0, task.vert_cache1, clip_count, task.out_stride);
				if (clip_count >= 3)
				{
					viewport_transform(clip_result, clip_count, task.out_stride);
					draw_triangles(clip_result, clip_count, task);
				}
			}
		}
		//const float *vert = task.in_vertex;
		//const float *end = vert + task.in_size * task.in_stride;
		//float *vert_out = task.out_vertex;
		//size_t vcnt = 0;
		//std::pair<Imath::V4f*, float*> clip_result;
		//for (; vert != end; vert += task.in_stride)
		//{
		//	task.program->vertex_shader(vert, vert_out, task.clip_pos[vcnt++]);
		//	if (vcnt < 3) {
		//		vert_out += task.out_stride;
		//		continue;
		//	}
		//	clip_result = clip_polygon_stream(task.clip_pos, task.clip_out,
		//		task.out_vertex, task.out_cache, vcnt, task.out_stride, 10);
		//	if (vcnt >= 3) {
		//		viewport_transform(clip_result.first, vcnt);
		//		draw_triangles(clip_result.first, clip_result.second, vcnt, task);
		//	}
		//	// next triangle
		//	vcnt = 0;
		//	vert_out = task.out_vertex;
		//}
	}

	void CSpwPipeline::viewport_transform(float* vert_pos, size_t size, size_t stride) const
	{
		for (size_t i = 0; i < size; ++i, vert_pos += stride)
		{
			Vec4f &pos = *reinterpret_cast<Vec4f*>(vert_pos);
			// we keep pos.w to correct interpolation
			// perspective projection: pos.w == -pos.z 
			// orthographic projection: pos.w == 1
			pos.x = m_vp_translate.x + m_vp_scale.x * (pos.x / pos.w);
			pos.y = m_vp_translate.y + m_vp_scale.y * (pos.y / pos.w);
			pos.z /= pos.w;
		}
	}

	//void CSpwPipeline::viewport_transform(Imath::V4f* vertex_pos, size_t size) const
	//{
	//	for (size_t i = 0; i < size; ++i)
	//	{
	//		auto &pos = vertex_pos[i];
	//		// we keep pos.w to correct interpolation
	//		// perspective projection: pos.w == -pos.z 
	//		// orthographic projection: pos.w == 1
	//		pos.x = m_vp_translate.x + m_vp_scale.x * (pos.x / pos.w);
	//		pos.y = m_vp_translate.y + m_vp_scale.y * (pos.y / pos.w);
	//		pos.z /= pos.w;
	//	}
	//}

	void CSpwPipeline::draw_triangles(float *vertices, size_t count, RasterTask &task) const
	{
		Vec4f *p0 = (Vec4f*)vertices, *p1 = (Vec4f*)(vertices + task.out_stride), *p2 = (Vec4f*)(vertices + task.out_stride * 2);
		auto center = task.block.center();
		p0->x -= center.x;
		p0->y -= center.y;
		p1->x -= center.x;
		p1->y -= center.y;
		Imath::Box2i bounding;
		CSpwPlotter plt(m_rt.get(), task, center);
		for (size_t j = 2; j < count; ++j)
		{
			p2->x -= center.x;
			p2->y -= center.y;
			// backface culling
			Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
			Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
			if (v10.cross(v12) * m_clock_wise > 0) {
				//if (m_draw_mode == LINE_MODE)
				//{
				//	draw_triangle_frame(*p0, *p1, *p2, plt);
				//	continue;
				//}
				// calculate triangle bounding box and intersection of region block
				Imath::bounding(bounding, p0, p1, p2);
				Imath::intersection(bounding, task.block);
				if (!bounding.isEmpty()) {
					plt.v0 = (float*)(p0 + 1);
					plt.v1 = (float*)(p1 + 1);
					plt.v2 = (float*)(p2 + 1);
					fill_triangle(bounding, *p0, *p1, *p2, plt);
				}
			}
			// next one
			p1 = p2;
			p2 = (Vec4f*)(((float*)p2) + task.out_stride);
			//p2 += 1;
		}
	}

	CSpwPipeline::CSpwPlotter::CSpwPlotter(CSpwRenderTarget *rt, RasterTask &task, const Imath::V2i &center)
		: m_rt(rt)
		, m_task(task)
		, m_center(center)
	{
		m_center.y = rt->height() - m_center.y - 1;
	}

	// fill mode
	void CSpwPipeline::CSpwPlotter::operator() (int x, int y, float z, float w1, float w2, float w3)
	{
		// todo: z-test first

		// interpolate vertex attributes
		const float *i0 = v0, *i1 = v1, *i2 = v2;
		for (float *out = m_task.frag_cache, *end = m_task.frag_cache + m_task.frag_stride; out != end; ++out, ++i0, ++i1, ++i2)
		{
			*out = *i0 * w1 + *i1 * w2 + *i2 * w3;
		}
		// write render target
		Imath::C4f frag_color;
		if (!m_task.material->fragment_shader(m_task.frag_cache, frag_color))
			return;
		// write fragment buffer
		frag_color.r *= frag_color.a;
		frag_color.g *= frag_color.a;
		frag_color.b *= frag_color.a;
		unsigned v = Imath::rgb2packed(frag_color);
		x += m_center.x;
		y = m_center.y - y;
		auto &surf = m_rt->get_color_buffer();
		//unsigned v2 = *surf.get<unsigned>(x, y);
		//assert(v2 == 0xff000000);
		surf.set(x, y, v);
	}

	// plot mode
	void CSpwPipeline::CSpwPlotter::operator() (int x, int y)
	{
		// write render target
		Imath::C4f frag_color;
		if (!m_task.material->fragment_shader(m_task.frag_cache, frag_color))
			return;
		frag_color.r *= frag_color.a;
		frag_color.g *= frag_color.a;
		frag_color.b *= frag_color.a;
		x += m_center.x;
		y = m_center.y - y;
		unsigned v = Imath::rgb2packed(frag_color);
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, v);
	}

} // namesace wyc
