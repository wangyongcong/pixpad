#pragma once

#include <vector>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include "mathfwd.h"
#include "floatmath.h"

namespace wyc
{
	template<class T>
	inline T deg2rad(T angle)
	{
		return T(angle * M_PI / 180.0);
	}

	template<class T>
	inline T rad2deg(T rad)
	{
		return T(rad * 180.0 / M_PI);
	}

	// OpenGL orthograph matrix
	void set_orthograph(Matrix44f &proj, float left_plane, float bottom_plane, float near_plane,
		float right_plane, float top_plane, float far_plane);

	// OpenGL perspective matrix
	// yfov: vertical FOV angle in degree
	// aspect: width/height
	// near_plane/far_plane: distance of near plane and far plane
	void set_perspective(Matrix44f &proj, float yfov, float aspect, float near_plane, float far_plane);

	//
	// 2D homogeneous transform matrix
	//
	// translate (x, y)
	void set_translate(Matrix33f &m, float x, float y);
	// scale (sx, sy)
	void set_scale(Matrix33f &m, float sx, float sy);
	// rotate radian counter-clockwise
	void set_rotate(Matrix33f &m, float radian);

	//
	// 3D homogeneous transform matrix
	//
	// translate (x, y, z)
	void set_translate(Matrix44f &m, float x, float y, float z);
	// scale (sx, sy, sz)
	void set_scale(Matrix44f &m, float sx, float sy, float sz);
	// rotate radian counter-clockwise around the normalized vector N
	void set_rotate(Matrix44f &m, const Vec3f &n, float radian);
	// rotate radian counter-clockwise around +X axis (1, 0, 0)
	void set_rotate_x(Matrix44f &m, float radian);
	// rotate radian counter-clockwise around +Y axis (0, 1, 0)
	void set_rotate_y(Matrix44f &m, float radian);
	// rotate radian counter-clockwise around +Z axis (0, 0, 1)
	void set_rotate_z(Matrix44f &m, float radian);

} // namespace wyc