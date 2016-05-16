#pragma once
#include <OpenEXR/ImathBox.h>

IMATH_INTERNAL_NAMESPACE_HEADER_ENTER

template<typename Point>
void bounding(Box2i &bounding, const Point *p0, const Point *p1, const Point *p2)
{
	float x, y;
	x = p0->x;
	if (x > p1->x)
		x = p1->x;
	if (x > p2->x)
		x = p2->x;
	y = p0->y;
	if (y > p1->y)
		y = p1->y;
	if (y > p2->y)
		y = p2->y;
	bounding.min = {
		int(std::floorf(x)),
		int(std::floorf(y))
	};
	x = p0->x;
	if (x < p1->x)
		x = p1->x;
	if (x < p2->x)
		x = p2->x;
	y = p0->y;
	if (y < p1->y)
		y = p1->y;
	if (y < p2->y)
		y = p2->y;
	bounding.max = {
		int(std::ceilf(x)),
		int(std::ceilf(y))
	};
}

template <class T>
void intersection(Box<T> &left, const Box<T> &other)
{
	left.min.x = std::max(left.min.x, other.min.x);
	left.min.y = std::max(left.min.y, other.min.y);
	left.max.x = std::min(left.max.x, other.max.x);
	left.max.y = std::min(left.max.y, other.max.y);
}

IMATH_INTERNAL_NAMESPACE_HEADER_EXIT
