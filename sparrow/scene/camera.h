#pragma once
#include <OpenEXR/ImathMatrix.h>
#include "mathfwd.h"
#include "scene_obj.h"

namespace wyc
{
	class CCamera : public CSceneObj
	{
	public:
		CCamera();
		~CCamera();
		void update_transform();
		void set_orthographic(float x_range_radius, float y_range_radius, float znear, float zfar);
		void set_perspective(float fov, float aspect, float znear, float zfar);
		const Matrix44f& get_projection() const;
		const Matrix44f& get_view_transform() const;
		const Matrix44f& get_view_projection() const;

	private:
		Matrix44f m_transform;
		Matrix44f m_projection;
		Matrix44f m_view_transform;
		Matrix44f m_view_proj;
	};

	inline const Matrix44f & CCamera::get_projection() const
	{
		return m_projection;
	}

	inline const Matrix44f & CCamera::get_view_transform() const
	{
		return m_view_transform;
	}

	inline const Matrix44f & CCamera::get_view_projection() const
	{
		return m_view_proj;
	}
} // namespace wyc