#pragma once
#include <OpenEXR/ImathMatrix.h>
#include "mathex/mathfwd.h"

namespace wyc
{
	class CCamera
	{
	public:
		CCamera();
		~CCamera();
		void set_orthographic(float x_range_radius, float y_range_radius, float znear, float zfar);
		void set_perspective(float fov, float aspect, float znear, float zfar);
		void set_transform(const Matrix44f& transform);
		const Matrix44f& get_transform() const;
		Matrix44f& get_transform();

	private:
		Matrix44f m_transform;
		Matrix44f m_projection;
	};

	inline void CCamera::set_transform(const Matrix44f & transform)
	{
		m_transform = transform;
	}

	inline const Matrix44f & CCamera::get_transform() const
	{
		return m_transform;
	}

	inline Matrix44f & CCamera::get_transform()
	{
		return m_transform;
	}

} // namespace wyc