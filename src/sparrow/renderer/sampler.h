#pragma once
#include <ImathVec.h>
#include <ImathColor.h>
#include "vecmath.h"
#include "image.h"

namespace wyc
{
	class CSampler
	{
	public:
		virtual ~CSampler() {}
		virtual void sample2d(const vec2f &uv, color4f &color) = 0;
		virtual void sample2d(const vec2f &uv, uint8_t level, color4f &color) = 0;
	};

	class CSpwSampler : public CSampler
	{
	public:
		CSpwSampler(const CImage *image);
		virtual ~CSpwSampler();
		virtual void sample2d(const vec2f &uv, color4f &color) override;
		virtual void sample2d(const vec2f &uv, uint8_t level, color4f &color) override;

	protected:
		const CImage *m_image;
	};

	class CSpwMipmapSampler : public CSampler
	{
	public:
		typedef std::vector<std::shared_ptr<CImage>> ImageVector;
		CSpwMipmapSampler(const ImageVector &mipmap_images);
		CSpwMipmapSampler(ImageVector &&mipmap_images);
		virtual void sample2d(const vec2f &uv, color4f &color) override;
		virtual void sample2d(const vec2f &uv, uint8_t level, color4f &color) override;

	protected:
		ImageVector m_images;
	};

} // namespace wyc