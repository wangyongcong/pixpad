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
		typedef VertexP3C3 vertex_t;
		void setup(const ConstantBuffer &const_buff);
		void feed(const CMesh &mesh);
		void stage_vertex(const CVertexBuffer &vb, size_t beg, size_t end);

	protected:
		unsigned m_num_core;
		Imath::M44f m_mvp;
	};

} // namespace wyc
