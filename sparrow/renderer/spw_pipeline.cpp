#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <OpenEXR/ImathColorAlgo.h>
#include <future>
#include "disruptor.h"
#include "ImathBoxAlgoExt.h"
#include "vecmath.h"
#include "spw_rasterizer.h"
#include "clipping.h"
#include "metric.h"

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
		//clear_async();
		process_async(mesh, material);
		return;

		assert(mesh && material);
		const CVertexBuffer &vb = mesh->vertex_buffer();
		const CIndexBuffer &ib = mesh->index_buffer();
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

		RasterTask task;
		task.material = material;
		task.index_stream = ib.data();
		task.index_size = ib.size() / 3 * 3;
		task.in_stream = &attrib_stream[0];
		task.in_count = attrib_stream.size();
		task.in_stride = attrib_def.in_stride;
		task.out_count = attrib_def.out_count;
		task.out_stride = attrib_def.out_stride;
		// assign surface block
		task.block = { { -halfw, -halfh },{ halfw, halfh } };
		task.block_center = { halfw, halfh };
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

	class CTile {
	public:
		CSpwRenderTarget *rt;
		const CMaterial *mtl;
		Imath::Box2i bounding;
		Imath::V2i center;
		const float *v0, *v1, *v2;
		std::vector<float> frag_cache;
		CTile(CSpwRenderTarget *prt, Imath::Box2i &b, Imath::V2i &c, const CMaterial *pmtl, size_t out_stride)
			: rt(prt)
			, mtl(pmtl)
			, bounding(b)
			, center(c)
			, v0(nullptr)
			, v1(nullptr)
			, v2(nullptr)
		{
			if (out_stride > 4) {
				size_t cache_frag = out_stride - 4;
				frag_cache.resize(cache_frag, 0);
			}
		}
		// plot mode
		void operator() (int x, int y) {

		}
		// fill mode
		void operator() (int x, int y, float z, float w1, float w2, float w3) {
			// todo: z-test first

			// interpolate vertex attributes
			float *fragment = frag_cache.data();
			const float *i0 = v0, *i1 = v1, *i2 = v2;
			for (float *out = fragment, *end = fragment + frag_cache.size(); out != end; ++out, ++i0, ++i1, ++i2)
			{
				*out = *i0 * w1 + *i1 * w2 + *i2 * w3;
			}
			// write render target
			Imath::C4f frag_color;
			if (!mtl->fragment_shader(fragment, frag_color))
				return;
			// write fragment buffer
			frag_color.r *= frag_color.a;
			frag_color.g *= frag_color.a;
			frag_color.b *= frag_color.a;
			unsigned v = Imath::rgb2packed(frag_color);
			x += center.x;
			y = rt->height() - (y + center.y) - 1;
			auto &surf = rt->get_color_buffer();
			//unsigned v2 = *surf.get<unsigned>(x, y);
			//assert(v2 == 0xff000000);
			surf.set(x, y, v);
		}

		void clear(const Imath::C3f &c)
		{
			Imath::Box2i b = bounding;
			b.min += center;
			b.max += center;
			int h = rt->height();
			unsigned v = Imath::rgb2packed(c);
			unsigned bg = Imath::rgb2packed(Color3f{ 0, 0, 0});
			auto &surf = rt->get_color_buffer();
			for (auto y = b.min.y; y < b.max.y; ++y) {
				for (auto x = b.min.x; x < b.max.x; ++x) {
					auto ty = h - y - 1;
					//assert(bg == *surf.get<unsigned>(x, ty));
					surf.set(x, ty, v);
				}
			}
		}
	};

	void CSpwPipeline::process_async(const CMesh * mesh, const CMaterial * material)
	{
		assert(mesh && material);
		const CVertexBuffer &vb = mesh->vertex_buffer();
		const CIndexBuffer &ib = mesh->index_buffer();
		// setup render target
		unsigned surfw, surfh;
		m_rt->get_size(surfw, surfh);
		int halfw = surfw >> 1, halfh = surfh >> 1;

		// bind stream
		auto &attrib_def = material->get_attrib_define();
		if (!check_material(attrib_def))
			return;

		constexpr unsigned NUM_VERTEX_CORES = 4;
		constexpr unsigned NUM_FRAGMENT_CORES = 4;
		unsigned triangle_count = ib.size() / 3;
		unsigned index_per_core = (triangle_count / NUM_VERTEX_CORES) * 3;
		unsigned beg = 0, end = index_per_core + (triangle_count % NUM_VERTEX_CORES) * 3;

		std::vector<const float*> attribs(attrib_def.in_count, nullptr);
		for (unsigned i = 0; i < attrib_def.in_count; ++i)
		{
			auto &slot = attrib_def.in_attribs[i];
			if (!vb.has_attribute(slot.usage)
				|| vb.attrib_component(slot.usage) < slot.component)
				return;
			attribs[i] = (const float*)vb.attrib_stream(slot.usage);
		}
		unsigned vertex_stride = vb.vertex_component();
		unsigned out_stride = attrib_def.out_stride;

		struct Event {
			CACHE_LINE_ALIGN std::vector<float> vec;
			std::vector<int> indices;
		};

		constexpr int BUFF_SIZE = 32;
		disruptor::ring_buffer<Event, BUFF_SIZE> out_buff;
		for (auto i = 0; i < out_buff.size(); ++i)
		{
			auto &e = out_buff.at(i);
			e.vec.resize(out_stride * 3, 0);
			//e.indices.reserve(8);
		}
		auto sw = std::make_shared<disruptor::shared_write_cursor>(BUFF_SIZE);
		std::vector<disruptor::read_cursor_ptr> readers(NUM_FRAGMENT_CORES);

		for (auto i = 0; i < NUM_FRAGMENT_CORES; ++i) {
			auto r = std::make_shared<disruptor::read_cursor>();
			r->follows(sw);
			sw->follows(r);
			readers[i] = r;
		}

		// generate vertex processors
		std::vector<std::future<void>> producers;
		while (end <= ib.size())
		{
			producers.push_back(std::async(std::launch::async, [this, beg, end, &ib, &attribs, vertex_stride, &material, out_stride, sw, &out_buff] {
				auto attrib_count = attribs.size();
				std::vector<const float*> attrib_ptrs(attrib_count, nullptr);
				auto vertex_in = &attrib_ptrs[0];
				// use triangle as the basic primitive (3 vertex)
				// clipping may produce 7 more vertex
				// so the maximum vertex count is 10
				constexpr int max_count = 10;
				// cache for vertex attributes 
				size_t cache_vert = out_stride * max_count;
				// we use double buffer to swap input/output
				float *vert_cache0 = new float[cache_vert + cache_vert];
				float *vert_cache1 = vert_cache0 + cache_vert;
				float *vertex_out = vert_cache0;
				std::vector<int> indices;
				indices.reserve(max_count - 3 + 1);
				int vcnt = 0;
				for (auto i = beg; i < end; ++i)
				{
					auto offset = ib[i] * vertex_stride;
					for (size_t j = 0; j < attrib_count; ++j)
					{
						attrib_ptrs[j] = attribs[j] + offset;
					}
					material->vertex_shader(vertex_in, vertex_out);
					vertex_out += out_stride;
					vcnt += 1;
					if (vcnt == 3) {
						size_t clip_count = 3;
						float *clip_result = clip_polygon_stream(vert_cache0, vert_cache1, clip_count, out_stride);
						if (clip_count >= 3)
						{
							viewport_transform(clip_result, clip_count, out_stride);
							Vec4f *p0 = (Vec4f*)clip_result, *p1 = (Vec4f*)(clip_result + out_stride), *p2 = (Vec4f*)(clip_result + out_stride * 2);
							for (size_t j = 2; j < clip_count; ++j)
							{
								// backface culling
								Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
								Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
								if (v10.cross(v12) * m_clock_wise > 0) {
									indices.push_back(0);
									indices.push_back(j - 1);
									indices.push_back(j);
								}
								// next one
								p1 = p2;
								p2 = (Vec4f*)(((float*)p2) + out_stride);
							}
							if (!indices.empty()) {
								auto pos = sw->claim(1);
								auto &e = out_buff.at(pos);
								// TODO: maybe swap the pointer, no copy
								e.vec.assign(clip_result, clip_result + clip_count * out_stride);
								e.indices.assign(indices.begin(), indices.end());
								sw->publish_after(pos, pos - 1);
								indices.clear();
							}
						}
						vcnt = 0;
						vertex_out = vert_cache0;
					}
				} // end of for-loop
				delete[] vert_cache0;
			}));
			beg = end;
			end += index_per_core;
		}

		// generate fragement processors
		constexpr int TILE_W = 32, TILE_H = 32;
		constexpr int HALF_TILE_W = TILE_W >> 1, HALF_TILE_H = TILE_H >> 1;
		int tile_x = (surfw + TILE_W - 1) / TILE_W, tile_y = (surfh + TILE_H - 1) / TILE_H;
		int margin_x = surfw & (TILE_W - 1), margin_y = surfh & (TILE_H - 1);
		std::vector<CTile> tiles;
		for (auto i = 0; i < tile_y; ++i) {
			for (auto j = 0; j < tile_x; ++j)
			{
				Imath::Box2i bounding = { {-HALF_TILE_W, -HALF_TILE_H}, {HALF_TILE_W, HALF_TILE_H} };
				Imath::V2i center = { HALF_TILE_W + j * TILE_W, HALF_TILE_H + i * TILE_H };
				tiles.emplace_back(m_rt.get(), bounding, center, material, out_stride);
			}
			// last column
			if (margin_x > 0) 
			{
				tiles.back().bounding.max.x -= TILE_W - margin_x;
			}
		}
		// last row
		if (margin_y > 0) {
			for (auto i = tiles.size() - tile_x, end = tiles.size(); i < end; ++i)
			{
				tiles[i].bounding.max.y -= TILE_H - margin_y;
			}
		}
		int tile_per_core = tiles.size() / NUM_FRAGMENT_CORES;
		int tile_beg = 0, tile_end = tile_per_core + tiles.size() % NUM_FRAGMENT_CORES;
		std::vector<std::future<void>> consumers;
		for (auto i = 0; i < NUM_FRAGMENT_CORES; ++i) {
			auto cursor = readers[i];
			consumers.push_back(std::async(std::launch::async, [this, cursor, &out_buff, out_stride, &tiles, tile_beg, tile_end] {
				Imath::Box2i vertex_bounding;
				auto beg = cursor->begin();
				auto end = cursor->end();
				Imath::V4f v0, v1, v2;
				while (1) {
					if (beg == end)
					{
						if (end > 0)
							cursor->publish(end - 1);
						end = cursor->wait_for(end);
					}
					auto &ev = out_buff.at(beg);
					const float* vec = ev.vec.data();
					if (std::isnan(vec[0]))
						break; // EOF
					const int *indices = ev.indices.data();
					for (size_t idx = 2; idx < ev.indices.size(); idx += 3) {
						int i1 = ev.indices[idx - 2];
						int i2 = ev.indices[idx - 1];
						int i3 = ev.indices[idx];
						// input stream is accessed by multiple fragment processors, must be read only
						const float *v = vec + i1 * out_stride;
						const Vec4f *p0 = (Vec4f*)v;
						v = vec + i2 * out_stride;
						const Vec4f *p1 = (Vec4f*)v;
						v = vec + i3 * out_stride;
						const Vec4f *p2 = (Vec4f*)v;
						v0 = *p0;
						v1 = *p1;
						v2 = *p2;
						Imath::bounding(vertex_bounding, p0, p1, p2);
						for (auto i = tile_beg; i < tile_end; ++i) {
							auto &tile = tiles[i];
							Imath::Box2i local_bounding = vertex_bounding;
							local_bounding.min -= tile.center;
							local_bounding.max -= tile.center;
							Imath::intersection(local_bounding, tile.bounding);
							if (!local_bounding.isEmpty()) {
								// fill triangles
								v0.x = p0->x - tile.center.x;
								v0.y = p0->y - tile.center.y;
								v1.x = p1->x - tile.center.x;
								v1.y = p1->y - tile.center.y;
								v2.x = p2->x - tile.center.x;
								v2.y = p2->y - tile.center.y;
								tile.v0 = (float*)p0;
								tile.v1 = (float*)p1;
								tile.v2 = (float*)p2;
								fill_triangle(local_bounding, v0, v1, v2, tile);
							} // bounding not empty
						} // tile loop
					} // index loop
					++beg;
				}
			}));
			tile_beg = tile_end;
			tile_end += tile_per_core;
		}

		// wait for producers
		for (auto &h : producers)
		{
			h.get();
		}

		// use NAN to indicate EOF
		auto pos = sw->claim(1);
		out_buff.at(pos).vec.assign(1, NAN);
		sw->publish_after(pos, pos - 1);

		// wait for consumers
		for (auto &h : consumers)
		{
			h.get();
		}
	}

	void CSpwPipeline::clear_async()
	{
		constexpr int NUM_FRAGMENT_CORES = 4;
		unsigned surfw, surfh;
		m_rt->get_size(surfw, surfh);
		// generate fragement processors
		constexpr int TILE_W = 32, TILE_H = 32;
		constexpr int HALF_TILE_W = TILE_W >> 1, HALF_TILE_H = TILE_H >> 1;
		int tile_x = (surfw + TILE_W - 1) / TILE_W, tile_y = (surfh + TILE_H - 1) / TILE_H;
		int margin_x = surfw & (TILE_W - 1), margin_y = surfh & (TILE_H - 1);
		std::vector<CTile> tiles;
		for (auto i = 0; i < tile_y; ++i) {
			for (auto j = 0; j < tile_x; ++j)
			{
				Imath::Box2i bounding = { { -HALF_TILE_W, -HALF_TILE_H },{ HALF_TILE_W, HALF_TILE_H } };
				Imath::V2i center = { HALF_TILE_W + j * TILE_W, HALF_TILE_H + i * TILE_H };
				tiles.emplace_back(m_rt.get(), bounding, center, nullptr, 0);
			}
			// last column
			if (margin_x > 0)
			{
				tiles.back().bounding.max.x -= TILE_W - margin_x;
			}
		}
		// last row
		if (margin_y > 0) {
			for (auto i = tiles.size() - tile_x, end = tiles.size(); i < end; ++i)
			{
				tiles[i].bounding.max.y -= TILE_H - margin_y;
			}
		}
		int tile_per_core = tiles.size() / NUM_FRAGMENT_CORES;
		int tile_beg = 0, tile_end = tile_per_core + tiles.size() % NUM_FRAGMENT_CORES;
		std::vector<std::future<void>> consumers;
		constexpr int COLOR_COUNT = 6;
		const Imath::C3f colors[COLOR_COUNT] = {
			{ 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },
			{ 1, 1, 0 },{ 1, 0, 1 },{ 0, 1, 1 },
		};
		TIMER_BEG(2)
		for (auto c = 0; c < NUM_FRAGMENT_CORES; ++c) {
			consumers.push_back(std::async(std::launch::async, [this, &tiles, tile_beg, tile_end, &colors, COLOR_COUNT] {
				for (auto i = tile_beg; i < tile_end; ++i)
				{
					auto &tile = tiles[i];
					tile.clear(colors[i % COLOR_COUNT]);
				}
			}));
			tile_beg = tile_end;
			tile_end += tile_per_core;
		}
		// wait for consumers
		for (auto &h : consumers)
		{
			h.get();
		}
		TIMER_END
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
		const unsigned *index_stream = task.index_stream;
		std::vector<const char*> attrib_ptr(task.in_count, nullptr);
		const void* in_vertex = &attrib_ptr[0];
		for (size_t i = 0; i < task.index_size; ++i)
		{
			auto idx_vert = index_stream[i];
			for (size_t j = 0; j < task.in_count; ++j)
			{
				auto &stream = task.in_stream[j];
				attrib_ptr[j] = stream.first + stream.second * idx_vert;
			}
			size_t vcnt = i % 3;
			COUNT_VERTEX
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

	void CSpwPipeline::draw_triangles(float *vertices, size_t count, RasterTask &task) const
	{
		Vec4f *p0 = (Vec4f*)vertices, *p1 = (Vec4f*)(vertices + task.out_stride), *p2 = (Vec4f*)(vertices + task.out_stride * 2);
		//log_debug("triangle: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", p0->x, p0->y, p0->z, p1->x, p1->y, p1->z, p2->x, p2->y, p2->z);
		auto center = task.block_center;
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
				// calculate triangle bounding box and intersection of region block
				Imath::bounding(bounding, p0, p1, p2);
				Imath::intersection(bounding, task.block);
				if (!bounding.isEmpty()) {
					plt.v0 = (float*)(p0);
					plt.v1 = (float*)(p1);
					plt.v2 = (float*)(p2);
					fill_triangle(bounding, *p0, *p1, *p2, plt);
				}
			}
			// next one
			p1 = p2;
			p2 = (Vec4f*)(((float*)p2) + task.out_stride);
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
		COUNT_FRAGMENT
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
