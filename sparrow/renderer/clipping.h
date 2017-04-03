#pragma once
#include <vector>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathBox.h>
#include "mathfwd.h"
#include "floatmath.h"

namespace wyc
{
	// Represent a plane in the form {normal, D} where D = dot(-point, normal)
	// point: a point on the plane
	// normal: the plane normal
	// we can calculate the signed distance between a point C to the plane efficiently: d = dot(C, plane)
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

	void intersect(float *v1, float d1, float *v2, float d2, size_t stride, float *out);

	// Clip polygon by planes
	// planes: planes used to clip the polygon
	// vertices: vertices of the polygon, when return, it will contain the result
	void clip_polygon(const std::vector<Vec4f> &planes, std::vector<Vec3f> &vertices);

	// Clip polygon in homogeneous clipping space
	void clip_polygon_homo(std::vector<Vec4f> &vertices);

	// Clip polygon in homogeneous clipping space
	// the polygon is treated as float stream
	// vertex_in: polygon vertices
	// vertex_out: vertices buffer for clipping
	// vertex_count: count of input vertices
	// stride: vertex stride (in number of float)
	// pos_offset: offset of vertex position of vertex struct
	// cache_size: size of vertex_in and vertex_out (in number of float)
	// vertex_in and vertex_out must be with the same size, and has enough space for clipping result
	// retutn: the buffer which contains the clipping result, it's either vertex_in or vertex_out.
	// return nullptr if the polygon is totally clipped out
	// when return, vertex_count specify the count of resulting vertices.
	float* clip_polygon_stream(float *vertex_in, float *vertex_out, size_t &vertex_count, size_t stride);

	// Clip polygon in homogeneous clipping space. Each polygon is in the form of float stream
	void clip_polygon_stream(std::vector<float> &vertices, std::vector<unsigned> &indices_in, std::vector<unsigned> &indices_out, unsigned stride);

	bool clip_line(Imath::V2f &v0, Imath::V2f &v1, const Imath::Box2f &clip_window);
} // namespace wyc