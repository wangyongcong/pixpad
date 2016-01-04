#pragma once

#include <vector>
#include "OpenEXR/ImathMatrix.h"
#include "mathfwd.h"
#include "floatmath.h"

namespace wyc
{

	template<class T>
	inline T deg2rad(T angle)
	{
		return T(angle * M_PI / 180.0);
	}

	template<typename T>
	inline T operator ^ (const Imath::Vec3<T> &v1, const Imath::Vec4<T> &v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
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

	// Represent a plane using vector4
	//	point: a point on the plane
	//	normal: the plane normal
	inline Imath::V4f plane(const Imath::V3f &point, const Imath::V3f &normal)
	{
		return Imath::V4f(normal.x, normal.y, normal.z, -point.dot(normal));
	}

	template<class Vec>
	inline Vec intersect(const Vec &p1, float d1, const Vec &p2, float d2)
	{
		float t = d1 / (d1 - d2);
		if (d1 < 0)
			t = fast_ceil(t * 1000) * 0.001f;
		else
			t = fast_floor(t * 1000) * 0.001f;
		return p1 + (p2 - p1) * t;
	}

	// Clip polygon by planes
	// planes: planes used to clip the polygon
	// vertices: vertices of the polygon, when return, it will contain the result
	void clip_polygon(const std::vector<Vec4f> &planes, std::vector<Vec3f> &vertices);

	// Clip polygon in homogeneous clip space
	void clip_polygon_homo(std::vector<Vec4f> &vertices);

} // namespace wyc