#pragma once
#include <OpenEXR/ImathNamespace.h>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathBox.h>

IMATH_INTERNAL_NAMESPACE_HEADER_ENTER

template<typename T>
inline T operator ^ (const Imath::Vec3<T> &v1, const Imath::Vec4<T> &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template<typename T>
inline bool inside(const Imath::Vec2<T> &point, const Imath::Box<Imath::Vec2<T>> &box)
{
	return (point.x >= box.min.x && point.y < box.max.x
		&& point.y >= box.min.y && point.y < box.max.y);
}

template<typename T>
inline bool inside(T x, T y, const Imath::Box<Imath::Vec2<T>> &box)
{
	return (x >= box.min.x && y < box.max.x
		&& y >= box.min.y && y < box.max.y);
}

IMATH_INTERNAL_NAMESPACE_HEADER_EXIT
