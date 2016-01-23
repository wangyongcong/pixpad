#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>
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

	class IShaderProgram
	{
	public:
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Imath::V4f &clip_pos) const = 0;
		virtual void fragment_shader(const float *vertex_out, float *frag_out) const = 0;
		virtual size_t get_vertex_stride() const = 0;
		virtual size_t get_vertex_size() const = 0;
	};

	class CSpwPipeline
	{
	public:
		CSpwPipeline();
		~CSpwPipeline();
		CSpwPipeline(const CSpwPipeline &other) = delete;
		CSpwPipeline& operator = (const CSpwPipeline &other) = delete;
		
		void setup(std::shared_ptr<CSpwRenderTarget> rt);
		void feed(const CMesh *mesh, const IShaderProgram *program);
		struct TaskVertex {
			const float *in_vertex;
			size_t in_size;
			size_t in_stride;
			const Imath::V3f *in_pos;
			size_t pos_stride;

			size_t out_stride;
			size_t out_offset_pos;

			char *cache;
			size_t cache_size;
			float *out_vertex;
			float *out_cache;
			Imath::V4f *clip_pos;
			Imath::V4f *clip_out;

			const IShaderProgram *program;
		};
		void process(TaskVertex &stream) const;

		inline void set_viewport(const Imath::Box2i &view) {
			auto _tmp = view.max + view.min;
			m_viewport_center = { _tmp.x * 0.5f, _tmp.y * 0.5f };
			_tmp = view.max - view.min;
			m_viewport_radius = { _tmp.x * 0.5f, _tmp.y * 0.5f };
		}
		void write_fragment(int x, int y, float *in, const IShaderProgram *program);

	protected:
		//static VertexOut* clip_polygon(VertexOut *in, VertexOut *out, size_t &size, size_t max_size);
		void viewport_transform(Imath::V4f* vertex_pos, size_t size) const;
		void draw_triangles(const Imath::V4f* vertex_pos, const float *vertices, size_t count, size_t stride, const IShaderProgram *program) const;

		unsigned m_num_core;
		POLYGON_WINDING m_clock_wise;
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_viewport_center;
		Imath::V2f m_viewport_radius;

		struct RasterRegion
		{
			Imath::V2i center;
			Imath::V2i center_device;
			Imath::Box2i block;
			Imath::Box2i block_device;
		};
		RasterRegion m_region;
	};

	class CShaderFlatColor : public IShaderProgram
	{
	public:
		typedef VertexP3C3 VertexIn;
		typedef VertexP3C3 VertexOut;
		typedef struct
		{
			Imath::C3f color;
		} Fragment;

		struct Uniform
		{
			Imath::M44f mvp;
		} m_uniform;

		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Imath::V4f &clip_pos) const override
		{
			const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
			VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
			Imath::V4f pos(in->pos);
			pos.z = -1.0f;
			pos = m_uniform.mvp * pos;
			clip_pos = pos;
			out->pos = in->pos;
			out->color = in->color;
		}

		virtual void fragment_shader(const float *vertex_out, float *frag_out) const override
		{
			const VertexOut* vout = reinterpret_cast<const VertexOut*>(vertex_out);
			Fragment *frag = reinterpret_cast<Fragment*>(frag_out);
			frag->color = vout->color;
		}

		virtual size_t get_vertex_stride() const override
		{
			return VertexOut::Layout::component;
		}
		virtual size_t get_vertex_size() const override
		{
			return sizeof(VertexOut);
		}
	};

} // namespace wyc
