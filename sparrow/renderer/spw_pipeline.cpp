#include "spw_pipeline.h"
#include <cassert>
#include <functional>
#include <future>
#include "disruptor.h"
#include "ImathBoxAlgoExt.h"
#include "vecmath.h"
#include "spw_rasterizer.h"
#include "platform_info.h"
#include "clipping.h"
#include "metric.h"

namespace wyc
{
	CSpwPipeline::CSpwPipeline()
		: m_is_setup(false)
		, m_num_vertex_unit(1)
		, m_num_fragment_unit(1)
		, m_clock_wise(COUNTER_CLOCK_WISE)
	{
	}

	CSpwPipeline::~CSpwPipeline()
	{
		m_rt = nullptr;
	}

	void CSpwPipeline::setup()
	{
		if (m_is_setup)
			return;
		m_is_setup = true;
		auto ncore = wyc::core_num();
		if (MAX_CORE_NUM > 0 && ncore > MAX_CORE_NUM)
			ncore = MAX_CORE_NUM;
		if (ncore > 1) {
			m_num_vertex_unit = ncore >> 1;
			m_num_fragment_unit = ncore - m_num_vertex_unit;
		}
		else {
			m_num_vertex_unit = 1;
			m_num_fragment_unit = 1;
		}
		m_prim_writer = std::make_shared<disruptor::shared_write_cursor>(PRIMITIVE_QUEUE_SIZE);
		for (int i = 0; i < m_num_fragment_unit; ++i) {
			auto ptr = std::make_shared<disruptor::read_cursor>();
			m_prim_readers.push_back(ptr);
			ptr->follows(m_prim_writer);
			m_prim_writer->follows(ptr);
		}
	}

	void CSpwPipeline::set_render_target(std::shared_ptr<CSpwRenderTarget> rt)
	{
		m_rt = rt;
		unsigned surfw, surfh;
		rt->get_size(surfw, surfh);
		set_viewport({ { 0, 0 },{ int(surfw), int(surfh) } });

		// split frame buffer into tiles
		static_assert((SPW_TILE_W & (SPW_TILE_W - 1)) == 0, "tile width should be pow of 2");
		static_assert((SPW_TILE_H & (SPW_TILE_H - 1)) == 0, "tile height should be pow of 2");
		constexpr int HALF_TILE_W = SPW_TILE_W >> 1, HALF_TILE_H = SPW_TILE_H >> 1;
		constexpr int MASK_TILW_W = SPW_TILE_W - 1, MASK_TILE_H = SPW_TILE_H - 1;
		int tile_x = (surfw + MASK_TILW_W) / SPW_TILE_W, tile_y = (surfh + MASK_TILE_H) / SPW_TILE_H;
		int margin_x = surfw & MASK_TILW_W, margin_y = surfh & MASK_TILE_H;
		Imath::Box2i tile_bounding = { { -HALF_TILE_W, -HALF_TILE_H },{ HALF_TILE_W, HALF_TILE_H } };
		for (auto i = 0; i < tile_y; ++i) {
			for (auto j = 0; j < tile_x; ++j)
			{
				Imath::V2i center = { HALF_TILE_W + j * SPW_TILE_W, HALF_TILE_H + i * SPW_TILE_H };
				m_tiles.emplace_back(m_rt.get(), tile_bounding, center);
			}
			// adjust last column tiles' bounding
			if (margin_x > 0)
			{
				m_tiles.back().bounding.max.x -= SPW_TILE_W - margin_x;
			}
		}
		// adjust last row tiles' bounding
		if (margin_y > 0) {
			for (auto i = m_tiles.size() - tile_x, end = m_tiles.size(); i < end; ++i)
			{
				m_tiles[i].bounding.max.y -= SPW_TILE_H - margin_y;
			}
		}
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
		//process(mesh, material);
	}

	void CSpwPipeline::process_async(const CMesh * mesh, const CMaterial * material)
	{
		assert(mesh && material);
		const CVertexBuffer &vb = mesh->vertex_buffer();
		const CIndexBuffer &ib = mesh->index_buffer();
		// setup render target
		unsigned surfw, surfh;
		m_rt->get_size(surfw, surfh);

		// bind stream
		auto &attrib_def = material->get_attrib_define();
		if (!check_material(attrib_def))
			return;
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
		unsigned output_stride = attrib_def.out_stride;

		// generate vertex processors
		unsigned triangle_count = ib.size() / 3;
		unsigned index_per_core = (triangle_count / m_num_vertex_unit) * 3;
		unsigned index_beg = 0, index_end = index_per_core + (triangle_count % m_num_vertex_unit) * 3;
		std::vector<std::future<void>> producers;
		while (index_end <= ib.size())
		{
			producers.push_back(std::async(std::launch::async, [this, &attribs, &ib, index_beg, index_end, vertex_stride, material, output_stride] {
				auto attrib_count = attribs.size();
				const float **vertex_in = new float const*[attrib_count];
				// use triangle as the basic primitive (3 vertex)
				// clipping may produce 7 more vertex
				// so the maximum vertex count is 10
				constexpr int max_count = 10;
				// cache for vertex attributes 
				size_t cache_vert = output_stride * max_count;
				std::vector<float> vertex_out;
				vertex_out.reserve(cache_vert);
				std::vector<unsigned> indices_in, indices_out;
				indices_in.reserve(max_count);
				indices_out.reserve(max_count);

				for (auto i = index_beg; i < index_end; ++i)
				{
					auto offset = ib[i] * vertex_stride;
					for (size_t j = 0; j < attrib_count; ++j)
					{
						vertex_in[j] = attribs[j] + offset;
					}
					auto cur_vert = vertex_out.size();
					vertex_out.resize(cur_vert + output_stride);
					material->vertex_shader(vertex_in, &vertex_out[cur_vert]);
					indices_in.push_back(cur_vert);
					if (indices_in.size() < 3)
						continue;
					if (!cull_backface(vertex_out, output_stride)) {
						clip_polygon_stream(vertex_out, indices_in, indices_out, output_stride);
						if (indices_out.size() >= 3)
						{
							viewport_transform(vertex_out, indices_out);
							// publish primitive
							if (!indices_out.empty()) {
								auto pos = m_prim_writer->claim(1);
								auto &prim = m_prim_queue.at(pos);
								prim.vertices.clear();
								for (auto j : indices_out)
								{
									auto beg = &vertex_out[j];
									prim.vertices.insert(prim.vertices.end(), beg, beg + output_stride);
								}
								prim.stride = output_stride;
								prim.material = material;
								m_prim_writer->publish_after(pos, pos - 1);
								indices_out.clear();
							}
						} // publish primitive
					} // backface culling
					indices_in.clear();
					vertex_out.clear();
				} // end of for-loop
				delete[] vertex_in;
			}));
			index_beg = index_end;
			index_end += index_per_core;
		}

		// generate fragement processors
		int tile_per_core = m_tiles.size() / m_num_fragment_unit;
		int tile_beg = 0, tile_end = tile_per_core + m_tiles.size() % m_num_fragment_unit;
		for (auto &tile : m_tiles) {
			tile.set_fragment(output_stride, material);
		}
		std::vector<std::future<void>> consumers;
		for(auto &cursor: m_prim_readers) {
			consumers.push_back(std::async(std::launch::async, [this, cursor, tile_beg, tile_end, output_stride] {
				Imath::Box2i vertex_bounding;
				auto beg = cursor->begin();
				auto end = cursor->end();
				while (1) {
					if (beg == end)
					{
						if (end > 0)
							cursor->publish(end - 1);
						end = cursor->wait_for(end);
					}
					const auto &prim = m_prim_queue.at(beg);
					const float* vec = prim.vertices.data();
					if (std::isnan(vec[0]))
						break; // EOF
					const float* end = vec + prim.vertices.size();
					const Imath::V4f *p0 = (const Imath::V4f*)vec;
					vec += prim.stride;
					const Imath::V4f *p1 = (const Imath::V4f*)vec;
					vec += prim.stride;
					const Imath::V4f *p2 = (const Imath::V4f*)vec;
					while (vec < end)
					{
						Imath::bounding(vertex_bounding, p0, p1, p2);
						for (auto i = tile_beg; i < tile_end; ++i) {
							auto &tile = m_tiles[i];
							Imath::Box2i local_bounding = vertex_bounding;
							local_bounding.min -= tile.center;
							local_bounding.max -= tile.center;
							Imath::intersection(local_bounding, tile.bounding);
							if (!local_bounding.isEmpty()) {
								// fill triangles
								Imath::V3f v0, v1, v2;
								v0.x = p0->x - tile.center.x;
								v0.y = p0->y - tile.center.y;
								v0.z = p0->z;
								v1.x = p1->x - tile.center.x;
								v1.y = p1->y - tile.center.y;
								v1.z = p1->z;
								v2.x = p2->x - tile.center.x;
								v2.y = p2->y - tile.center.y;
								v2.z = p2->z;
								tile.set_triangle((float*)p0, (float*)p1, (float*)p2);
								fill_triangle(local_bounding, v0, v1, v2, tile);
							} // bounding not empty
						} // tile loop
						vec += prim.stride;
						p1 = p2;
						p2 = (const Imath::V4f*)vec;
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
		auto pos = m_prim_writer->claim(1);
		m_prim_queue.at(pos).vertices.assign(1, NAN);
		m_prim_writer->publish_after(pos, pos - 1);

		// wait for consumers
		for (auto &h : consumers)
		{
			h.get();
		}
	}

	void CSpwPipeline::clear_async()
	{
		int tile_per_core = m_tiles.size() / m_num_fragment_unit;
		int tile_beg = 0, tile_end = tile_per_core + m_tiles.size() % m_num_fragment_unit;
		std::vector<std::future<void>> consumers;
		constexpr int COLOR_COUNT = 6;
		const Imath::C3f colors[COLOR_COUNT] = {
			{ 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 },
			{ 1, 1, 0 },{ 1, 0, 1 },{ 0, 1, 1 },
		};
		for (auto c = 0; c < m_num_fragment_unit; ++c) {
			consumers.push_back(std::async(std::launch::async, [this, tile_beg, tile_end, &colors, COLOR_COUNT] {
				for (auto i = tile_beg; i < tile_end; ++i)
				{
					auto &tile = m_tiles[i];
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
	}

	void CSpwPipeline::process(const CMesh *mesh, const CMaterial *material) const
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
		std::vector<const float*> attribs;
		attribs.resize(attrib_def.in_count);
		for (unsigned i = 0; i < attrib_def.in_count; ++i)
		{
			auto &slot = attrib_def.in_attribs[i];
			if (!vb.has_attribute(slot.usage)
				|| vb.attrib_component(slot.usage) < slot.component)
				return;
			attribs[i] = (const float*)vb.attrib_stream(slot.usage);
		}

		unsigned vertex_stride = vb.vertex_component();
		unsigned output_stride = attrib_def.out_stride;
		auto attrib_count = attribs.size();
		const float **vertex_in = new float const*[attrib_count];
		// use triangle as the basic primitive (3 vertex)
		// clipping may produce 7 more vertex
		// so the maximum vertex count is 10
		constexpr int max_count = 10;
		// cache for vertex attributes 
		size_t cache_vert = output_stride * max_count;
		std::vector<float> vertex_out;
		vertex_out.reserve(cache_vert);
		std::vector<unsigned> indices_in, indices_out;
		indices_in.reserve(max_count);
		indices_out.reserve(max_count);
		CTile tile(m_rt.get(), Imath::Box2i{ { -halfw, -halfh },{ halfw, halfh } }, Imath::V2i{ halfw, halfh });
		tile.set_fragment(output_stride, material);
		for (size_t i = 0; i < ib.size(); ++i)
		{
			auto offset = ib[i] * vertex_stride;
			for (size_t j = 0; j < attrib_count; ++j)
			{
				vertex_in[j] = attribs[j] + offset;
			}
			auto cur_vert = vertex_out.size();
			vertex_out.resize(cur_vert + output_stride);
			material->vertex_shader(vertex_in, &vertex_out[cur_vert]);
			indices_in.push_back(cur_vert);
			if (indices_in.size() < 3)
				continue;
			if (!cull_backface(vertex_out, output_stride)) {
				clip_polygon_stream(vertex_out, indices_in, indices_out, output_stride);
				if (indices_out.size() >= 3)
				{
					viewport_transform(vertex_out, indices_out);
					draw_triangles(vertex_out, indices_out, output_stride, tile);
				}
			}
			indices_in.clear();
			vertex_out.clear();
		} // index buffer loop

		delete[] vertex_in;
	}

	bool CSpwPipeline::cull_backface(const std::vector<float> &vertices, unsigned stride) const
	{
		// p = {x, y, z, w}
		const float *p0 = vertices.data();
		const float *p1 = p0 + stride;
		const float *p2 = p1 + stride;
		Imath::V2f v10(p0[0] - p1[0], p0[1] - p1[1]);
		Imath::V2f v12(p2[0] - p1[0], p2[1] - p1[1]);
		return v10.cross(v12) * m_clock_wise <= 0;
	}

	void CSpwPipeline::set_viewport(const Imath::Box2i & view)
	{
		auto _tmp = view.max + view.min;
		m_vp_translate = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		_tmp = view.max - view.min;
		m_vp_scale = { _tmp.x * 0.5f, _tmp.y * 0.5f };
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

	void CSpwPipeline::viewport_transform(std::vector<float>& vertices, const std::vector<unsigned>& indices) const
	{
		for (auto j : indices)
		{
			Imath::V4f *pos = reinterpret_cast<Imath::V4f*>(&vertices[j]);
			// we keep pos.w to correct interpolation
			// perspective projection: pos.w == -pos.z 
			// orthographic projection: pos.w == 1
			pos->x = m_vp_translate.x + m_vp_scale.x * (pos->x / pos->w);
			pos->y = m_vp_translate.y + m_vp_scale.y * (pos->y / pos->w);
			pos->z /= pos->w;
		}
	}

	void CSpwPipeline::draw_triangles(const std::vector<float>& vertices, const std::vector<unsigned>& indices, unsigned stride, CTile &tile) const
	{
		const float* vec = vertices.data();
		const float* end = vec + vertices.size();
		const Imath::V4f *p0 = (const Imath::V4f*)vec;
		vec += stride;
		const Imath::V4f *p1 = (const Imath::V4f*)vec;
		vec += stride;
		const Imath::V4f *p2 = (const Imath::V4f*)vec;
		Imath::Box2i local_bounding;
		while (vec < end)
		{
			Imath::bounding(local_bounding, p0, p1, p2);
			local_bounding.min -= tile.center;
			local_bounding.max -= tile.center;
			Imath::intersection(local_bounding, tile.bounding);
			if (!local_bounding.isEmpty()) {
				// fill triangles
				Imath::V3f v0, v1, v2;
				v0.x = p0->x - tile.center.x;
				v0.y = p0->y - tile.center.y;
				v0.z = p0->z;
				v1.x = p1->x - tile.center.x;
				v1.y = p1->y - tile.center.y;
				v1.z = p1->z;
				v2.x = p2->x - tile.center.x;
				v2.y = p2->y - tile.center.y;
				v2.z = p2->z;
				tile.set_triangle((float*)p0, (float*)p1, (float*)p2);
				fill_triangle(local_bounding, v0, v1, v2, tile);
			} // bounding not empty
			vec += stride;
			p1 = p2;
			p2 = (const Imath::V4f*)vec;
		} // index loop
	}

} // namesace wyc
