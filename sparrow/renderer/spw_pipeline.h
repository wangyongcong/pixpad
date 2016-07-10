#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include "ImathVecExt.h"
#include "ImathMatrixExt.h"
#include "mesh.h"
#include "vertex_layout.h"
#include "spw_render_target.h"
#include "material.h"

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
		typedef std::pair<const char*, size_t> AttribStream;
		struct RasterTask {
			const CMaterial *material;
			const char* index_stream;
			size_t index_size;
			size_t index_stride;
			const AttribStream* in_stream;
			size_t in_count;
			size_t in_stride;
			size_t out_count;
			size_t out_stride;  // output vertex stride (in float component)
			char *cache;  // cache buffer
			size_t cache_size;  // cache size in bytes
			float *vert_cache0;  // output vertex cache 1
			float *vert_cache1;  // output vertex cache 2
			//Vec4f *clip_cache0;  // clip position cache 1
			//Vec4f *clip_cache1;  // clip position cache 2
			float *frag_cache;
			size_t frag_stride;
			Imath::Box2i block;
			Imath::V2i block_center;
		};
		bool check_material(const AttribDefine &attrib_def) const;
		void process(RasterTask &stream) const;
		void viewport_transform(float* vert_pos, size_t size, size_t stride) const;
		void draw_triangles(float *vertices, size_t count, RasterTask &task) const;
		//void viewport_transform(Imath::V4f* vertex_pos, size_t size) const;
		//void draw_triangles(Imath::V4f* vertex_pos, const float *vertices, size_t count, RasterTask &task) const;

		unsigned m_num_core;
		POLYGON_WINDING m_clock_wise;
		POLYGON_DRAW_MODE m_draw_mode;
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_vp_translate;
		Imath::V2f m_vp_scale;

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
	};

} // namespace wyc
