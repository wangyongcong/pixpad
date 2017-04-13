#pragma once
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "image.h"

namespace wyc
{
	class CSampler
	{
	public:
		virtual ~CSampler() {}
		virtual void sample2d(const Imath::V2f &uv, Imath::C4f &color) = 0;
	};

	class CSpwSampler : public CSampler
	{
	public:
		CSpwSampler(const CImage *image);
		virtual ~CSpwSampler();
		virtual void sample2d(const Imath::V2f &uv, Imath::C4f &color) override;

	private:
		const CImage *m_image;
	};

} // namespace wyc