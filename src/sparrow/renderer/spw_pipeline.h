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
		void setup(unsigned max_core=MAX_CORE_NUM);
		void set_render_target(std::shared_ptr<CSpwRenderTarget> rt);
		virtual void feed(const CMesh *mesh, const CMaterial *material);
		void set_viewport(const Imath::Box2i &view);

	protected:
		typedef std::pair<const char*, size_t> AttribStream;
		POLYGON_WINDING m_clock_wise;
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_vp_translate;
		Imath::V2f m_vp_scale;
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

		bool check_material(const AttribDefine &attrib_def) const;
		virtual void process(const CMesh *mesh, const CMaterial *material) const;
		virtual void process_async(const CMesh *mesh, const CMaterial *material);
		void clear_async();
		void viewport_transform(std::vector<float> &vertices, const std::vector<unsigned> &indices) const;
		bool cull_backface(const std::vector<float> &vertices, unsigned stride) const;
		virtual void draw_triangles(const std::vector<float> &vertices, const std::vector<unsigned> &indices, unsigned stride, CTile &tile) const;
	};

} // namespace wyc
