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

namespace wyc
{
	class CTile {
	public:
		CSpwRenderTarget *rt;
		const CMaterial *mtl;
		Imath::Box2i bounding;
		Imath::V2i center;
		const float *v0, *v1, *v2;
		std::vector<float> frag_cache;
		CTile(CSpwRenderTarget *prt, Imath::Box2i &b, Imath::V2i &c, const CMaterial *pmtl = nullptr)
			: rt(prt)
			, mtl(pmtl)
			, bounding(b)
			, center(c)
			, v0(nullptr)
			, v1(nullptr)
			, v2(nullptr)
		{
		}
		void set_fragment_input_size(size_t stride)
		{
			if (stride > 4) {
				size_t cache_frag = stride - 4;
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
			unsigned bg = Imath::rgb2packed(Color3f{ 0, 0, 0 });
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
		bool cull_backface(const std::vector<float> &vertices, unsigned stride) const;
		virtual void draw_triangles(float *vertices, size_t count, RasterTask &task) const;

		// async render
		bool m_is_setup;
		int m_num_vertex_unit;
		int m_num_fragment_unit;
		struct CACHE_LINE_ALIGN Primitive {
			std::vector<float> vertices;
			std::vector<int> indices;
			int stride;
		};
		disruptor::ring_buffer<Primitive, PRIMITIVE_QUEUE_SIZE> m_prim_queue;
		disruptor::shared_write_cursor_ptr m_prim_writer;
		std::vector<disruptor::read_cursor_ptr> m_prim_readers;
		std::vector<CTile> m_tiles;

		void clear_async();
		void process_async(const CMesh *mesh, const CMaterial *material);
	};

} // namespace wyc
