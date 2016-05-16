#include "spw_texture.h"
#include <cassert>
#include <OpenEXR/ImathColor.h>
#include <mathex/floatmath.h>

namespace wyc
{
	CSpwTexture2D::CSpwTexture2D()
		: m_image(nullptr)
	{
	}

	CSpwTexture2D::CSpwTexture2D(std::shared_ptr<CImage> image)
		: m_image(nullptr)
	{
		set_image(image);
	}

	CSpwTexture2D::CSpwTexture2D(const CSpwTexture2D & other)
		: m_image(other.m_image)
	{
	}

	CSpwTexture2D::~CSpwTexture2D()
	{
		m_image = nullptr;
	}

	CSpwTexture2D& CSpwTexture2D::operator = (const CSpwTexture2D &other)
	{
		m_image = other.m_image;
		return *this;
	}

	void CSpwTexture2D::set_image(std::shared_ptr<CImage> image)
	{
		m_image = image;
	}

	Imath::Color4<float> CSpwTexture2D::sample(const Imath::Vec2<float> &uv) const
	{
		assert(m_image != nullptr);
		int x = fast_round(uv.x * m_image->width());
		int y = fast_round(uv.y * m_image->height());
		return m_image->get_color(x, y);
	}


} // namespace wyc