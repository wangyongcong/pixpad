#include "camera.h"
#include "mathex/vecmath.h"

namespace wyc
{
	CCamera::CCamera()
	{
	}

	CCamera::~CCamera()
	{
	}

	void CCamera::set_orthographic(float x_range_radius, float y_range_radius, float znear, float zfar)
	{
		wyc::set_orthograph(m_projection, -x_range_radius, -y_range_radius, znear,
			x_range_radius, y_range_radius, zfar);
	}

	void CCamera::set_perspective(float fov, float aspect, float znear, float zfar)
	{
		wyc::set_perspective(m_projection, fov, aspect, znear, zfar);
	}

} // namespace wyc