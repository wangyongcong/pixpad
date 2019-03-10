#pragma once
#include <cmath>
#include <ImathPlatform.h>
#include <ImathForward.h>

namespace wyc
{
	typedef Imath::Vec2<float> vec2f;
	typedef Imath::Vec3<float> vec3f;
	typedef Imath::Vec4<float> vec4f;
	typedef Imath::Vec2<int> vec2i;
	typedef Imath::Vec3<int> vec3i;
	typedef Imath::Vec4<int> vec4i;
	typedef Imath::Matrix33<float> mat3f;
	typedef Imath::Matrix44<float> mat4f;
	typedef Imath::Matrix33<int> mat3i;
	typedef Imath::Matrix44<int> mat4i;
	typedef Imath::Box<vec2f> box2f;
	typedef Imath::Box<vec3f> box3f;
	typedef Imath::Box<vec2i> box2i;
	typedef Imath::Box<vec3i> box3i;
	typedef Imath::Color3<float> color3f;
	typedef Imath::Color4<float> color4f;

	inline float deg2rad(float angle)
	{
		return float(angle * M_PI / 180.0);
	}

	inline float rad2deg(float rad)
	{
		return float(rad * 180.0 / M_PI);
	}

	// OpenGL orthograph matrix
	void set_orthograph(mat4f &proj, float left_plane, float bottom_plane, float near_plane,
		float right_plane, float top_plane, float far_plane);

	// OpenGL perspective matrix
	// yfov: vertical FOV angle in degree
	// aspect: width/height
	// near_plane/far_plane: distance of near plane and far plane
	void set_perspective(mat4f &proj, float yfov, float aspect, float near_plane, float far_plane);

	//
	// 2D homogeneous transform matrix
	//
	// translate (x, y)
	void set_translate(mat3f &m, float x, float y);
	// scale (sx, sy)
	void set_scale(mat3f &m, float sx, float sy);
	// rotate radian counter-clockwise
	void set_rotate(mat3f &m, float radian);

	//
	// 3D homogeneous transform matrix
	//
	// translate (x, y, z)
	void set_translate(mat4f &m, float x, float y, float z);
	// scale (sx, sy, sz)
	void set_scale(mat4f &m, float sx, float sy, float sz);
	// rotate radian counter-clockwise around the normalized vector N
	void set_rotate(mat4f &m, const vec3f &n, float radian);
	// rotate radian counter-clockwise around +X axis (1, 0, 0)
	void set_rotate_x(mat4f &m, float radian);
	// rotate radian counter-clockwise around +Y axis (0, 1, 0)
	void set_rotate_y(mat4f &m, float radian);
	// rotate radian counter-clockwise around +Z axis (0, 0, 1)
	void set_rotate_z(mat4f &m, float radian);

} // namespace wyc
