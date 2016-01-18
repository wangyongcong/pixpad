#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include "mesh.h"
#include "vertex_layout.h"
#include "spw_render_target.h"

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
		~CSpwPipeline();
		CSpwPipeline(const CSpwPipeline &other) = delete;
		CSpwPipeline& operator = (const CSpwPipeline &other) = delete;
		
		void setup(std::shared_ptr<CSpwRenderTarget> rt);
		void feed(const CMesh &mesh);
		void process(const CVertexBuffer &vb, size_t beg, size_t end) const;

		typedef CVertexP4C3 VertexOut;
		typedef CVertexP3C3 VertexIn;
		typedef struct {
			Imath::C3f color;
		} Fragment;
		typedef struct
		{
			Imath::M44f mvp;
			Imath::V2f viewport_center;
			Imath::V2f viewport_radius;
		} Uniform;
		inline void set_uniform(const Uniform &uniform) {
			m_uniform = uniform;
		}
		inline void set_viewport(const Imath::Box2i &view) {
			auto _tmp = view.max + view.min;
			m_uniform.viewport_center = { _tmp.x * 0.5f, _tmp.y * 0.5f };
			_tmp = view.max - view.min;
			m_uniform.viewport_radius = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		}
		static void vertex_shader(const Uniform &uniform, const VertexIn &in, VertexOut &out);
		static void fragment_shader(const Uniform &uniform, const VertexOut &in, Fragment &out);

	protected:
		static VertexOut* clip_polygon(VertexOut *in, VertexOut *out, size_t &size, size_t max_size);
		void draw_triangles(const float *vertices, size_t count, size_t stride, size_t pos_offset) const;
		void write_fragment(int x, int y, VertexOut &in);

		unsigned m_num_core;
		Uniform m_uniform;
		POLYGON_WINDING m_clock_wise;
		std::shared_ptr<CSpwRenderTarget> m_rt;

		struct RasterRegion
		{
			Imath::V2i center;
			Imath::V2i center_device;
			Imath::Box2i block;
			Imath::Box2i block_device;
		};
		RasterRegion m_region;
	};

} // namespace wyc
