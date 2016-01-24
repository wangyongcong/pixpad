#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include "mathex/vecmath.h"
#include "spw_rasterizer.h"
#include "clipping.h"

namespace wyc
{
	CSpwPipeline::CSpwPipeline()
		: m_num_core(1)
		, m_clock_wise(COUNTER_CLOCK_WISE)
	{
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
	}

	void CSpwPipeline::feed(const CMesh *mesh, const IShaderProgram *program)
	{
		assert(mesh && program);
		const CVertexBuffer &vb = mesh->vertex_buffer();
#ifdef _DEBUG
		// check input vertex format
		if (!vb.has_attribute(ATTR_POSITION) || vb.attrib_component(ATTR_POSITION) < 3)
		{
			assert(0 && "Input vertex must contain 3D position.");
		}
#endif // _DEBUG

		// todo: work parallel
		TaskVertex task;
		task.program = program;
		task.in_vertex = vb.get_vertex_stream();
		task.in_size = vb.size();
		task.in_stride = vb.vertex_component();		
		task.in_pos = reinterpret_cast<const Imath::V3f*>(vb.get_attrib_stream(ATTR_POSITION));
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

		process(task);
	}

	void CSpwPipeline::set_viewport(const Imath::Box2i & view)
	{
		auto _tmp = view.max + view.min;
		m_viewport_center = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		_tmp = view.max - view.min;
		m_viewport_radius = { _tmp.x * 0.5f, _tmp.y * 0.5f };
	}

	void CSpwPipeline::process(TaskVertex &task) const
	{
		const float *vert = task.in_vertex;
		const float *end = vert + task.in_size * task.in_stride;
		float *vert_out = task.out_vertex;
		size_t vcnt = 0;
		std::pair<Imath::V4f*, float*> clip_result;
		for (; vert != end; vert += task.in_stride, vert_out += task.out_stride)
		{
			task.program->vertex_shader(vert, vert_out, task.clip_pos[vcnt++]);
			if (vcnt < 3)
				continue;
			// clipping
			clip_result = clip_polygon_stream(task.clip_pos, task.clip_out,
				task.out_vertex, task.out_cache, vcnt, task.out_stride, 10);
			if (!vcnt)
				continue;
			assert(vcnt >= 3);
			// viewport transform
			viewport_transform(clip_result.first, vcnt);
			// draw triangles
			draw_triangles(clip_result.first, clip_result.second, vcnt, task.out_stride, task.program);
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
			pos.x = m_viewport_center.x + m_viewport_radius.x * (pos.x / pos.w);
			pos.y = m_viewport_center.y + m_viewport_radius.y * (pos.y / pos.w);
			pos.z /= pos.w;
		}
	}

	class CSpwPlotter
	{
	public:
		const float *v0, *v1, *v2;
		CSpwPlotter(CSpwPipeline *pipeline, const IShaderProgram *program, size_t stride)
			: m_pipeline(pipeline)
			, m_program(program)
			, m_size(stride)
		{
			m_out = new float[m_size];
		}
		~CSpwPlotter()
		{
			delete[] m_out;
			m_out = nullptr;
		}

		void operator() (int x, int y, float z, float w1, float w2, float w3)
		{
			// todo: z-test first

			const float *i0 = v0, *i1 = v1, *i2 = v2;
			for (float *out = m_out, *end = m_out + m_size; out != end; ++out, ++i0, ++i1, ++i2)
			{
				*out = *i0 * w1 + *i1 * w2 + *i2 * w3;
			}
			m_pipeline->write_fragment(x, y, m_out, m_program);
		}

	private:
		CSpwPipeline *m_pipeline;
		const IShaderProgram *m_program;
		float *m_out;
		size_t m_size;
	};

	void CSpwPipeline::draw_triangles(Imath::V4f* vertex_pos, const float *vertices, size_t count, size_t stride, const IShaderProgram *program) const
	{
		CSpwPlotter plt(const_cast<CSpwPipeline*>(this), program, stride);
		plt.v0 = vertices;
		plt.v1 = plt.v0 + stride;
		plt.v2 = plt.v1 + stride;
		Imath::V4f *p0 = vertex_pos, *p1 = vertex_pos + 1, *p2 = vertex_pos + 2;
		p0->x -= m_region.center.x;
		p0->y -= m_region.center.y;
		p1->x -= m_region.center.x;
		p1->y -= m_region.center.y;
		for (size_t j = 2; j < count; ++j)
		{
			p2->x -= m_region.center.x;
			p2->y -= m_region.center.y;
			// backface culling
			Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
			Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
			if (v10.cross(v12) * m_clock_wise <= 0)
				return;
			// todo: calculate triangle bounding box, and intersect with region block
			fill_triangle(m_region.block, *p0, *p1, *p2, plt);
			// next one
			p1 = p2;
			p2 += 1;
			plt.v1 = plt.v2;
			plt.v2 += stride;
		}
	}

	void CSpwPipeline::write_fragment(int x, int y, float *in, const IShaderProgram *program)
	{
		Imath::C3f out;
		program->fragment_shader(in, (float*)&out);
		// write fragment buffer
		auto &surf = m_rt->get_color_buffer();
		unsigned v = Imath::rgb2packed(out);
		x += m_region.center_device.x;
		y = m_region.center_device.y - y;
		assert(inside(x, y, m_region.block_device));
		surf.set(x, y, v);
	}

} // namesace wyc
