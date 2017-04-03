#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathColorAlgo.h>
#include "ImathVecExt.h"
#include "ImathMatrixExt.h"
#include "mesh.h"
#include "vertex_layout.h"
#include "spw_config.h"
#include "spw_render_target.h"
#include "material.h"
#include "disruptor.h"
#include "tile.h"

namespace wyc
{
	enum POLYGON_WINDING
	{
		CLOCK_WISE = 1,
		COUNTER_CLOCK_WISE = -1
	};

	class CSpwPipeline
	{
	public:
		CSpwPipeline();
		virtual ~CSpwPipeline();
		CSpwPipeline(const CSpwPipeline &other) = delete;
		CSpwPipeline& operator = (const CSpwPipeline &other) = delete;
		void setup();
		void set_render_target(std::shared_ptr<CSpwRenderTarget> rt);
		virtual void feed(const CMesh *mesh, const CMaterial *material);
		void set_viewport(const Imath::Box2i &view);

	protected:
		typedef std::pair<const char*, size_t> AttribStream;
		POLYGON_WINDING m_clock_wise;
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_vp_translate;
		Imath::V2f m_vp_scale;

		struct RasterTask {
			const CMaterial *material;
			const unsigned* index_stream;
			size_t index_size;
			const AttribStream* in_stream;
			size_t in_count;
			size_t in_stride;
			size_t out_count;
			size_t out_stride;  // output vertex stride (in float component)
			char *cache;  // cache buffer
			size_t cache_size;  // cache size in bytes
			float *vert_cache0;  // output vertex cache 1
			float *vert_cache1;  // output vertex cache 2
			float *frag_cache;
			size_t frag_stride;
			Imath::Box2i block;
			Imath::V2i block_center;
		};
		class CSpwPlotter
		{
		public:
			const float *v0, *v1, *v2;
			CSpwPlotter(CSpwRenderTarget *rt, RasterTask &task, const Imath::V2i &center);
			// plot mode
			void operator() (int x, int y);
			// fill mode
			void operator() (int x, int y, float z, float w1, float w2, float w3);
		private:
			CSpwRenderTarget *m_rt;
			RasterTask &m_task;
			Imath::V2i m_center;
		};
		bool check_material(const AttribDefine &attrib_def) const;
		virtual void process(RasterTask &task) const;
		void viewport_transform(float* vert_pos, size_t size, size_t stride) const;
		void viewport_transform(std::vector<float> &vertices, const std::vector<unsigned> &indices) const;
		bool cull_backface(const std::vector<float> &vertices, unsigned stride) const;
		virtual void draw_triangles(float *vertices, size_t count, RasterTask &task) const;

		// async render
		bool m_is_setup;
		int m_num_vertex_unit;
		int m_num_fragment_unit;
		struct CACHE_LINE_ALIGN Primitive {
			std::vector<float> vertices;
			unsigned stride;
			const CMaterial *material;
		};
		disruptor::ring_buffer<Primitive, PRIMITIVE_QUEUE_SIZE> m_prim_queue;
		disruptor::shared_write_cursor_ptr m_prim_writer;
		std::vector<disruptor::read_cursor_ptr> m_prim_readers;
		std::vector<CTile> m_tiles;

		void clear_async();
		void process_async(const CMesh *mesh, const CMaterial *material);
	};

} // namespace wyc
