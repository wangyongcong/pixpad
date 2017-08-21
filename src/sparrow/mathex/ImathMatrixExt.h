#pragma once
#include <ImathNamespace.h>
#include <ImathMatrix.h>

IMATH_INTERNAL_NAMESPACE_HEADER_ENTER

template<typename S>
inline Imath::Vec4<S> operator * (const Imath::Matrix44<S> &m, const Imath::Vec4<S> &v)
{
	S x = S(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + v.w * m[0][3]);
	S y = S(v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + v.w * m[1][3]);
	S z = S(v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + v.w * m[2][3]);
	S w = S(v.x * m[3][0] + v.y * m[3][1] + v.z * m[3][2] + v.w * m[3][3]);

	return Imath::Vec4<S>(x, y, z, w);
}

// affine transform: matrix44 * vector4(x, y, z, 1)
template<typename S>
inline Imath::Vec3<S> operator * (const Imath::Matrix44<S> &m, const Imath::Vec3<S> &v)
{
	S x = S(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + m[0][3]);
	S y = S(v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + m[1][3]);
	S z = S(v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + m[2][3]);

	return Imath::Vec3<S>(x, y, z);
}


IMATH_INTERNAL_NAMESPACE_HEADER_EXIT
