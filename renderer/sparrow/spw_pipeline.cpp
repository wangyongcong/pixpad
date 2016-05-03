#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include <mathex/ImathBoxAlgoExt.h>
#include "mathex/vecmath.h"
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

	void CSpwPipeline::feed(const CMesh *mesh, const CMaterial *material)
	{
		assert(mesh && material);
		const CVertexBuffer &vb = mesh->vertex_buffer();
#ifdef _DEBUG
		// check input vertex format
		if (!vb.has_attribute(ATTR_POSITION) || vb.attrib_component(ATTR_POSITION) < 3)
		{
			assert(0 && "Input vertex must contain 3D position.");
			return;
		}
#endif // _DEBUG
		//const CIndexBuffer &ib = mesh->index_buffer();

		// setup render target
		unsigned surfw, surfh;
		m_rt->get_size(surfw, surfh);
		int halfw = surfw >> 1, halfh = surfh >> 1;

		// todo: work parallel
		TaskVertex task;
		task.material = material;
		task.in_vertex = vb.get_vertex_stream();
		task.in_size = vb.size();
		task.in_stride = vb.vertex_component();
		task.in_pos = reinterpret_cast<const Imath::V3f*>(vb.attrib_stream(ATTR_POSITION));
		size_t out_stride = program->get_vertex_stride();
		task.out_stride = out_stride;
		// use triangle as the basic primitive (3 vertex)
		// clipping may produce 7 more vertex
		// so the maximum vertex count is 10
		constexpr int max_count = 10;
		size_t cache_vert = out_stride * sizeof(float) * max_count * 2;
		// cache for clipping position (double buffer)
		size_t cache_clip = sizeof(Imath::V4f) * max_count * 2;
		task.cache_size = cache_vert + cache_clip;
		task.cache = new char[task.cache_size];
		task.out_vertex = reinterpret_cast<float*>(task.cache);
		task.out_cache = task.out_vertex + out_stride * max_count;
		task.clip_pos = reinterpret_cast<Imath::V4f*>(task.cache + cache_vert);
		task.clip_out = task.clip_pos + max_count;
		// assign surface block
		task.block = { {0, 0}, {halfw, halfh} };

		process(task);
	}

	void CSpwPipeline::set_viewport(const Imath::Box2i & view)
	{
		auto _tmp = view.max + view.min;
		m_vp_translate = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		_tmp = view.max - view.min;
		m_vp_scale = { _tmp.x * 0.5f, _tmp.y * 0.5f };
	}

	void CSpwPipeline::process(TaskVertex &task) const
	{
		const float *vert = task.in_vertex;
		const float *end = vert + task.in_size * task.in_stride;
		float *vert_out = task.out_vertex;
		size_t vcnt = 0;
		std::pair<Imath::V4f*, float*> clip_result;
		for (; vert != end; vert += task.in_stride)
		{
			task.program->vertex_shader(vert, vert_out, task.clip_pos[vcnt++]);
			if (vcnt < 3) {
				vert_out += task.out_stride;
				continue;
			}
			clip_result = clip_polygon_stream(task.clip_pos, task.clip_out,
				task.out_vertex, task.out_cache, vcnt, task.out_stride, 10);
			if (vcnt >= 3) {
				viewport_transform(clip_result.first, vcnt);
				draw_triangles(clip_result.first, clip_result.second, vcnt, task);
			}
			// next triangle
			vcnt = 0;
			vert_out = task.out_vertex;
		}
	}

	void CSpwPipeline::viewport_transform(Imath::V4f* vertex_pos, size_t size) const
	{
		for (size_t i = 0; i < size; ++i)
		{
			auto &pos = vertex_pos[i];
			// we keep pos.w to correct interpolation
			// perspective projection: pos.w == -pos.z 
			// orthographic projection: pos.w == 1
			pos.x = m_vp_translate.x + m_vp_scale.x * (pos.x / pos.w);
			pos.y = m_vp_translate.y + m_vp_scale.y * (pos.y / pos.w);
			pos.z /= pos.w;
		}
	}

	class CSpwPlotter
	{
	public:
		const float *v0, *v1, *v2;
		CSpwPlotter(CSpwRenderTarget *rt, const IShaderProgram *program, const Imath::Box2i &block, size_t commponent)
			: m_rt(rt)
			, m_program(program)
			, m_size(commponent)
		{
			m_vert = new float[m_size];
			m_center = block.center();
			m_center.y = rt->height() - m_center.y - 1;
		}
		~CSpwPlotter()
		{
			delete[] m_vert;
			m_vert = nullptr;
		}

		// plot mode
		void operator() (int x, int y)
		{
			// write render target
			Imath::C4f frag_color;
			if (!m_program->fragment_shader(m_vert, frag_color))
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

		// fill mode
		void operator() (int x, int y, float z, float w1, float w2, float w3)
		{
			// todo: z-test first

			const float *i0 = v0, *i1 = v1, *i2 = v2;
			for (float *out = m_vert, *end = m_vert + m_size; out != end; ++out, ++i0, ++i1, ++i2)
			{
				*out = *i0 * w1 + *i1 * w2 + *i2 * w3;
			}
			// write render target
			Imath::C4f frag_color;
			if (!m_program->fragment_shader(m_vert, frag_color))
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

	private:
		CSpwRenderTarget *m_rt;
		const IShaderProgram *m_program;
		float *m_vert;
		size_t m_size;
		Imath::V2i m_center;
	};

	void CSpwPipeline::draw_triangles(Imath::V4f* vertex_pos, const float *vertices, size_t count, TaskVertex &task) const
	{
		size_t stride = task.out_stride;
		auto center = task.block.center();
		CSpwPlotter plt(m_rt.get(), task.program, task.block, stride);
		plt.v0 = vertices;
		plt.v1 = plt.v0 + stride;
		plt.v2 = plt.v1 + stride;
		Imath::V4f *p0 = vertex_pos, *p1 = vertex_pos + 1, *p2 = vertex_pos + 2;
		p0->x -= center.x;
		p0->y -= center.y;
		p1->x -= center.x;
		p1->y -= center.y;
		Imath::Box2i bounding;
		for (size_t j = 2; j < count; ++j)
		{
			p2->x -= center.x;
			p2->y -= center.y;
			// backface culling
			Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
			Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
			if (v10.cross(v12) * m_clock_wise <= 0)
				return;
			if (m_draw_mode == LINE_MODE)
			{
				draw_triangle_frame(*p0, *p1, *p2, plt);
				continue;
			}
			// calculate triangle bounding box and intersection of region block
			Imath::bounding(bounding, p0, p1, p2);
			Imath::intersection(bounding, task.block);
			if (!bounding.isEmpty()) 
				fill_triangle(bounding, *p0, *p1, *p2, plt);
			// next one
			p1 = p2;
			p2 += 1;
			plt.v1 = plt.v2;
			plt.v2 += stride;
		}
	}

} // namesace wyc
