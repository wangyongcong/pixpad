#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>
#include "mesh.h"
#include "vertex_layout.h"
#include "spw_render_target.h"
#include <renderer/material.h>

namespace wyc
{
	enum POLYGON_WINDING
	{
		CLOCK_WISE = 1,
		COUNTER_CLOCK_WISE = -1
	};

	enum POLYGON_DRAW_MODE
	{
		FILL_MODE,
		LINE_MODE,
	};

	class CSpwPipeline
	{
	public:
		CSpwPipeline();
		~CSpwPipeline();
		CSpwPipeline(const CSpwPipeline &other) = delete;
		CSpwPipeline& operator = (const CSpwPipeline &other) = delete;
		void setup(std::shared_ptr<CSpwRenderTarget> rt);
		void feed(const CMesh *mesh, const CMaterial *program);
		void set_viewport(const Imath::Box2i &view);
		inline void set_draw_mode(POLYGON_DRAW_MODE m) {
			m_draw_mode = m;
		}

	protected:
		struct TaskVertex {
			const CMaterial *material;
			const float *in_vertex;
			size_t in_size;  // input vertex count
			size_t in_stride;  // input vertex stride (in float componenet)
			const Imath::V3f *in_pos;  // input vertex position array
			size_t out_stride;  // output vertex stride (in float component)
			char *cache;  // cache buffer
			size_t cache_size;  // cache size in bytes
			float *out_vertex;  // output vertex cache 1
			float *out_cache;   // output vertex cache 2
			Imath::V4f *clip_pos;  // clip position cache 1
			Imath::V4f *clip_out;  // clip output cache 2
			Imath::Box2i block;
		};
		void process(TaskVertex &stream) const;
		void viewport_transform(Imath::V4f* vertex_pos, size_t size) const;
		void draw_triangles(Imath::V4f* vertex_pos, const float *vertices, size_t count, TaskVertex &task) const;

		unsigned m_num_core;
		POLYGON_WINDING m_clock_wise;
		POLYGON_DRAW_MODE m_draw_mode;
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_vp_translate;
		Imath::V2f m_vp_scale;
	};

} // namespace wyc
