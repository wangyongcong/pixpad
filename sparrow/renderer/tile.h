#pragma once
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathColorAlgo.h>
#include "spw_render_target.h"
#include "material.h"

namespace wyc
{
	class CTile 
	{
	public:
		Imath::Box2i bounding;
		Imath::V2i center;
		const float *m_v0, *m_v1, *m_v2;
		CTile(CSpwRenderTarget *rt, Imath::Box2i &bounding, Imath::V2i &center);
		// setup triangle
		void set_triangle(const float* v0, const float* v1, const float* v2, unsigned vertex_stride, const CMaterial *material);
		// clear the tile
		void clear(const Imath::C3f &c);
		// plot mode
		void operator() (int x, int y);
		// fill mode
		void operator() (int x, int y, float z, float w1, float w2, float w3);

	private:
		CSpwRenderTarget *m_rt;
		const CMaterial *m_material;
		std::vector<float> m_fragment_input;
	};

} // namespace wyc