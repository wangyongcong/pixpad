#pragma once
#include <ImathBox.h>
#include <ImathColorAlgo.h>
#include "vecmath.h"
#include "material.h"
#include "spw_render_target.h"

namespace wyc
{
	class CTile 
	{
	public:
		box2i bounding;
		vec2i center;
		CTile(CSpwRenderTarget *rt, const box2i &bounding, const vec2i &center);
		// setup fragment
		void set_fragment(unsigned vertex_stride, const CMaterial *material);
		// setup triangle
		inline void set_triangle(const float* v0, const float* v1, const float* v2)
		{
			m_v0 = v0;
			m_v1 = v1;
			m_v2 = v2;
			m_inv_z0 = 1 / v0[3];
			m_inv_z1 = 1 / v1[3];
			m_inv_z2 = 1 / v2[3];
		}
		// clear the tile
		void clear(const color4f &c);
		// plot mode
		void operator() (int x, int y);
		// fill mode
		void operator() (int x, int y, float z, float w1, float w2, float w3);
		// fill by quad
		void operator() (int x, int y, const vec4f &z, const vec4i &is_inside,
			const vec4f &w1, const vec4f &w2, const vec4f &w3);

	private:
		void _interp(const vec4f & w1, const vec4f & w2, const vec4f & w3);
		void _interp_with_correction(const vec4f & w1, const vec4f & w2, const vec4f & w3);
		CSpwRenderTarget *m_rt;
		const CMaterial *m_material;
		const float *m_v0, *m_v1, *m_v2;
		float m_inv_z0, m_inv_z1, m_inv_z2;
		std::vector<float> m_frag_input;
		float *m_frag_interp[4];
		int m_transform_y;
		unsigned m_stride;
		CShaderContext m_ctx;
		bool m_correction;
	};

} // namespace wyc
