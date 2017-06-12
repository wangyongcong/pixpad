#include "sampler.h"
#include "floatmath.h"

namespace wyc
{
	CSpwSampler::CSpwSampler(const CImage *image)
		: m_image(image)
	{
	}

	CSpwSampler::~CSpwSampler()
	{
	}

	void CSpwSampler::sample2d(const Imath::V2f & uv, Imath::C4f & color)
	{
		float u, v, s, t;
		auto img_w = m_image->width();
		auto img_h = m_image->height();
		u = uv.x * img_w;
		v = uv.y * img_h;

		int x0, y0, x1, y1;
		x0 = fast_floor(u);
		y0 = fast_floor(v);
		u -= x0;
		v -= y0;

		x0 %= img_w;
		y0 %= img_h;
		x1 = (x0 + 1) % img_w;
		y1 = (y0 + 1) % img_h;

		Imath::C4f c1, c2, c3, c4;
		c1 = m_image->get_color(x0, y0);
		c2 = m_image->get_color(x1, y0);
		c3 = m_image->get_color(x1, y1);
		c4 = m_image->get_color(x0, y1);

		s = 1 - u;
		t = 1 - v;

		c1 *= s * t;
		c2 *= u * t;
		c3 *= u * v;
		c4 *= s * v;
		color = c1 + c2 + c3 + c4;
	}

} // namespace wyc
