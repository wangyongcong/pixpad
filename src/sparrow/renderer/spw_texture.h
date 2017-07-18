#pragma once
#include <memory>
#include <vector>
#include <OpenEXR/ImathForward.h>
#include "image.h"

namespace wyc
{
	class CSpwTexture
	{
	public:
		typedef std::shared_ptr<CImage> image_ptr;
		CSpwTexture();
		CSpwTexture(std::shared_ptr<CImage> image);
		CSpwTexture(const CSpwTexture &other);
		~CSpwTexture();
		CSpwTexture& operator = (const CSpwTexture &other);
		void set_image(std::shared_ptr<CImage> image);
		bool create_mipmap();
		inline Imath::Color4<float> sample(const Imath::Vec2<float> &uv) const {
			return sample(uv, 0);
		}
		Imath::Color4<float> sample(const Imath::Vec2<float> &uv, uint8_t level) const;
	private:
		std::vector<image_ptr> m_images;
	};

} // namepsace wyc