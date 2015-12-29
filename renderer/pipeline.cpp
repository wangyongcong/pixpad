#include "pipeline.h"
#include <OpenEXR/IlmThreadPool.h>
#include "mathex/vecmath.h"
#include "thread/platform_info.h"

namespace wyc
{
	CPipeline::CPipeline()
		: m_num_core(0)
	{
		if (0 == m_num_core)
		{
			// use as many cores as possible
			m_num_core = core_num();
		}
	}


	void CPipeline::feed(const CMesh &mesh)
	{
		const CVertexBuffer &vb = mesh.vertex_buffer();
		size_t vert_cnt = vb.size();
		size_t tri_cnt = vert_cnt / 3;
		//size_t tri_per_core = tri_cnt / m_num_core;
		
		// todo: work parallel
		stage_vertex(vb, 0, vert_cnt);
	}

	void CPipeline::stage_vertex(const CVertexBuffer &vb, size_t beg, size_t end)
	{
		auto it_beg = vb.begin();
		auto it_end = it_beg + end;
		std::vector<Imath::V4f> triangle;
		triangle.resize(3);
		unsigned char i = 0;
		for (auto it = it_beg + beg; it != it_end; ++it)
		{
			const vertex_t &v = *it;
			//---------------------------------
			// vertex shader
			Imath::V4f vs_out(v.pos);
			vs_out *= m_mvp;
			// return vs_out;
			// vertex shader end
			//---------------------------------
			triangle[i++] = vs_out;
			if (i == 3)
			{
				clip_polygon_homo(triangle);
				if (!triangle.empty())
				{
					viewport_transform(triangle);
					draw_triangle(triangle);
				}
			}

		}
	}


} // namesace wyc
