#include "sampler.h"
#include "floatmath.h"

namespace wyc
{
	CSpwSampler::CSpwSampler()
		: m_image(nullptr)
	{
	}

	CSpwSampler::~CSpwSampler()
	{
	}

	void CSpwSampler::sample2d(const Imath::V2f & uv, Imath::C4f & color)
	{
		float u, v, s, t;
		u = uv.x * (m_image->width() - 1);
		v = uv.y * (m_image->height() - 1);

		int x0, y0, x1, y1;
		x0 = fast_floor(u);
		y0 = fast_floor(v);
		x1 = fast_ceil(u);
		y1 = fast_ceil(v);

		Imath::C4f c1, c2, c3, c4;
		c1 = m_image->get_color(x0, y0);
		c2 = m_image->get_color(x1, y0);
		c3 = m_image->get_color(x1, y1);
		c4 = m_image->get_color(x0, y1);

		u -= x0;
		v -= y0;
		s = 1 - u;
		t = 1 - v;

		c1 *= s * t;
		c2 *= u * t;
		c3 *= u * v;
		c4 *= s * v;
		color = c1 + c2 + c3 + c4;
	}

} // namespace wyc
