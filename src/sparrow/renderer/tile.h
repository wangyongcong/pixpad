#pragma once
#include <ImathBox.h>
#include <ImathColorAlgo.h>
#include "spw_render_target.h"
#include "material.h"

namespace wyc
{
	class CTile 
	{
	public:
		Imath::Box2i bounding;
		Imath::V2i center;
		CTile(CSpwRenderTarget *rt, Imath::Box2i &bounding, Imath::V2i &center);
		// setup fragment
		void set_fragment(unsigned vertex_stride, const CMaterial *material);
		// setup triangle
		inline void CTile::set_triangle(const float* v0, const float* v1, const float* v2)
		{
			m_v0 = v0;
			m_v1 = v1;
			m_v2 = v2;
			m_inv_z0 = 1 / v0[3];
			m_inv_z1 = 1 / v1[3];
			m_inv_z2 = 1 / v2[3];
		}
		// clear the tile
		void clear(const Imath::C3f &c);
		// plot mode
		void operator() (int x, int y);
		// fill mode
		void operator() (int x, int y, float z, float w1, float w2, float w3);
		// fill by quad
		void operator() (int x, int y, const Imath::V4f &z, const Imath::V4i &is_inside,
			const Imath::V4f &w1, const Imath::V4f &w2, const Imath::V4f &w3);

	private:
		CSpwRenderTarget *m_rt;
		const CMaterial *m_material;
		const float *m_v0, *m_v1, *m_v2;
		float m_inv_z0, m_inv_z1, m_inv_z2;
		std::vector<float> m_frag_input;
		float *m_frag_interp[4];
		int m_transform_y;
		unsigned m_stride;
		CShaderContext m_ctx;
	};

} // namespace wyc