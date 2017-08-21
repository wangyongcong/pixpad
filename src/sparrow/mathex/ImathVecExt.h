#pragma once
#include <ImathNamespace.h>
#include <ImathVec.h>
#include <ImathBox.h>
#include <ImathExc.h>

IMATH_INTERNAL_NAMESPACE_HEADER_ENTER

template<class T>
inline T operator ^ (const Imath::Vec3<T> &v1, const Imath::Vec4<T> &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template<class T>
inline bool inside(const Imath::Vec2<T> &point, const Imath::Box<Imath::Vec2<T>> &box)
{
	return (point.x >= box.min.x && point.y < box.max.x
		&& point.y >= box.min.y && point.y < box.max.y);
}

template<class T>
inline bool inside(T x, T y, const Imath::Box<Imath::Vec2<T>> &box)
{
	return (x >= box.min.x && y < box.max.x
		&& y >= box.min.y && y < box.max.y);
}

//template<class T>
//inline Imath::Vec2<T> operator | (const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
//{
//	static_assert(std::is_integral<T>::value, "OR operation can only apply to integer vector");
//	return Imath::Vec2<T>{v1.x | v2.x, v1.y | v2.y};
//}
//
//template<class T>
//inline Imath::Vec3<T> operator | (const Imath::Vec3<T> &v1, const Imath::Vec3<T> &v2)
//{
//	static_assert(std::is_integral<T>::value, "OR operation can only apply to integer vector");
//	return Imath::Vec3<int>{v1.x | v2.x, v1.y | v2.y, v1.z | v2.z};
//}
//
//template<class T>
//inline Imath::Vec4<T> operator | (const Imath::Vec4<T> &v1, const Imath::Vec4<T> &v2)
//{
//	static_assert(std::is_integral<T>::value, "OR operation can only apply to integer vector");
//	return Imath::Vec4<int>{v1.x | v2.x, v1.y | v2.y, v1.z | v2.z, v1.w | v2.w};
//}

IMATH_INTERNAL_NAMESPACE_HEADER_EXIT
