#include "spw_texture.h"
#include <cassert>
#include <OpenEXR/ImathColor.h>
#include "floatmath.h"

namespace wyc
{
	CSpwTexture::CSpwTexture()
		: m_images()
	{
	}

	CSpwTexture::CSpwTexture(std::shared_ptr<CImage> image)
		: m_images()
	{
		set_image(image);
	}

	CSpwTexture::CSpwTexture(const CSpwTexture & other)
		: m_images(other.m_images)
	{
	}

	CSpwTexture::~CSpwTexture()
	{
	}

	CSpwTexture& CSpwTexture::operator = (const CSpwTexture &other)
	{
		m_images = other.m_images;
		return *this;
	}

	void CSpwTexture::set_image(std::shared_ptr<CImage> image)
	{
		m_images.clear();
		m_images.push_back(image);
	}

	bool CSpwTexture::create_mipmap()
	{
		if(m_images.empty())
			return false;
		auto img0 = m_images[0];
		auto w = img0->width();
		auto h = img0->height();
		if ((w & (w - 1)) || (h & (h - 1)))
			// size should be power of 2
			return false;
		m_images.resize(1);
		while (w > 1 && h > 1)
		{
			w >>= 1;
			h >>= 1;
			auto i = std::make_shared<CImage>();
			img0->resize(*i, w, h);
			m_images.push_back(i);
		}
		return true;
	}

	Imath::Color4<float> CSpwTexture::sample(const Imath::Vec2<float>& uv, uint8_t level) const
	{
		if(level >= m_images.size())
			return Imath::Color4<float>();
		auto img = m_images[level];
		int x = fast_round(uv.x * img->width());
		int y = fast_round(uv.y * img->height());
		return img->get_color(x, y);
	}


} // namespace wyc