#pragma once
#include <memory>
#include <OpenEXR/ImathForward.h>
#include "image.h"

namespace wyc
{
	class CSpwTexture2D
	{
	public:
		CSpwTexture2D();
		CSpwTexture2D(std::shared_ptr<CImage> image);
		CSpwTexture2D(const CSpwTexture2D &other);
		~CSpwTexture2D();
		CSpwTexture2D& operator = (const CSpwTexture2D &other);
		void set_image(std::shared_ptr<CImage> image);
		Imath::Color4<float> sample(const Imath::Vec2<float> &uv) const;
	private:
		std::shared_ptr<CImage> m_image;
	};

} // namepsace wyc