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
		virtual bool fragment_shader(const float *vertex_out, Imath::C4f &frag_color) const = 0;
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
		void set_viewport(const Imath::Box2i &view);

	protected:
		struct TaskVertex {
			const IShaderProgram *program;
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
		std::shared_ptr<CSpwRenderTarget> m_rt;
		Imath::V2f m_vp_translate;
		Imath::V2f m_vp_scale;
	};

	class CShaderFlatColor : public IShaderProgram
	{
	public:
		typedef VertexP3C3 VertexIn;
		typedef VertexP3C3 VertexOut;

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

		virtual bool fragment_shader(const float *vertex_out, Imath::C4f &frag_color) const override
		{
			const VertexOut* vout = reinterpret_cast<const VertexOut*>(vertex_out);
			frag_color = { vout->color.x, vout->color.y, vout->color.z, 1.0f };
			return true;
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
