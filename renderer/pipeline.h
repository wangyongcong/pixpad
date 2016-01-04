#pragma once
#include <OpenEXR/ImathMatrix.h>
#include "mesh.h"
#include "vertex_layout.h"

namespace wyc
{
	class CPipeline
	{
	public:
		CPipeline();
		~CPipeline();
		
		void feed(const CMesh &mesh);
		void stage_vertex(const CVertexBuffer &vb, size_t beg, size_t end);

		typedef VertexP4C3 VertexOut;
		typedef VertexP3C3 VertexIn;
		typedef Imath::C3f Fragment;
		typedef struct
		{
			Imath::M44f mvp;
			Imath::V2f viewport_center;
			Imath::V2f viewport_radius;
		} Uniform;
		inline void set_uniform(const Uniform &uniform) {
			m_uniform = uniform;
		}
		static void vertex_shader(const Uniform &uniform, const VertexIn &in, VertexOut &out);
		static void fragment_shader(const Uniform &uniform, const VertexOut &in, Fragment &out);

	protected:
		static VertexOut* clip_polygon(VertexOut *in, VertexOut *out, size_t &size, size_t max_size);
		static void viewport_transform(const Imath::V2f &center, const Imath::V2f &radius, VertexOut *in, size_t size);

		unsigned m_num_core;
		Uniform m_uniform;
	};

} // namespace wyc
