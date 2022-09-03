#include "sampler.h"
#include "floatmath.h"
#include <algorithm>

namespace wyc
{
	color4f bilinear_filter(const CImage *image, const vec2f &uv)
	{
		float u, v, s, t;
		auto img_w = image->width();
		auto img_h = image->height();
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

		color4f c1, c2, c3, c4;
		c1 = image->get_color(x0, y0);
		c2 = image->get_color(x1, y0);
		c3 = image->get_color(x1, y1);
		c4 = image->get_color(x0, y1);

		s = 1 - u;
		t = 1 - v;

		c1 *= s * t;
		c2 *= u * t;
		c3 *= u * v;
		c4 *= s * v;
		return color4f{ c1 + c2 + c3 + c4 };
	}

	CSpwSampler::CSpwSampler(const CImage *image)
		: m_image(image)
	{
	}

	CSpwSampler::~CSpwSampler()
	{
	}

	void CSpwSampler::sample2d(const vec2f & uv, color4f & color)
	{
		color = bilinear_filter(m_image, uv);
	}

	void CSpwSampler::sample2d(const vec2f & uv, uint8_t level, color4f & color)
	{
		color = bilinear_filter(m_image, uv);
	}

	CSpwMipmapSampler::CSpwMipmapSampler(const ImageVector & mipmap_images)
		: m_images(mipmap_images)
	{
	}

	CSpwMipmapSampler::CSpwMipmapSampler(ImageVector && mipmap_images)
		: m_images(mipmap_images)
	{
	}

	void CSpwMipmapSampler::sample2d(const vec2f & uv, color4f & color)
	{
		color = bilinear_filter(m_images[0].get(), uv);
	}

	void CSpwMipmapSampler::sample2d(const vec2f & uv, uint8_t level, color4f & color)
	{
		level = std::min<uint8_t>(level, uint8_t(m_images.size()) - 1);
		color = bilinear_filter(m_images[level].get(), uv);
	}

} // namespace wyc
