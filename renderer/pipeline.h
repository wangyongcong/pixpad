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
		void setup(const ConstantBuffer &const_buff);

		typedef VertexP3C3 vertex_t;
		void feed(const CMesh &mesh)
		{
			const CVertexBuffer &vb = mesh.vertex_buffer();
			vertex_t vert;
			auto pos_array = vb.get_attribute(ATTR_POSITION);
			auto color_array = vb.get_attribute(ATTR_COLOR);
			auto it_pos = pos_array.begin(), end_pos = pos_array.end();
			auto it_color = color_array.begin();
			for (; it_pos != end_pos; ++it_pos, ++it_color)
			{
				vert.pos = *it_pos;
				vert.color = *it_color;
				stage_geometry(vert);
			}
		}

		void stage_geometry(vertex_t &vert)
		{
			
		}

		void stage_raster(vertex_t &vert) {


		}

	protected:
		Imath::M44f m_mvp;
	};

} // namespace wyc
