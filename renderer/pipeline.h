#pragma once
#include "mesh.h"
#include "vertex_layout.h"

#include "OpenEXR/ImathMatrix.h"

namespace wyc
{
	class CPipeline
	{
	public:
		CPipeline();
		~CPipeline();
		
		struct ConstantBuffer
		{
			Imath::M44f mvp;
		};
		typedef VertexP4C3 VertexOut;
		typedef VertexP3C3 VertexIn;
		void setup(const ConstantBuffer &const_buff);
		void feed(const CMesh &mesh);
		void stage_vertex(const CVertexBuffer &vb, size_t beg, size_t end);
		void vertex_shader(const VertexIn &in, VertexOut &out);

	protected:
		VertexOut* clip_polygon(VertexOut *in, VertexOut *out, size_t &size);

		unsigned m_num_core;
		Imath::M44f m_mvp;
	};

} // namespace wyc
