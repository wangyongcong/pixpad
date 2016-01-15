#include <cassert>
#include "vecmath.h"

namespace wyc
{
	void set_orthograph(Matrix44f &proj, float l, float b, float n, float r, float t, float f)
	{
		proj = {
			2/(r-l),	0,			0,			(r+l)/(l-r),
			0,			2/(t-b),	0,			(t+b)/(b-t),
			0,			0,			2/(n-f),	(f+n)/(n-f),
			0,			0,			0,			1
		};
	}

	void set_perspective(Matrix44f &proj, float fov, float aspect, float n, float f)
	{
		n = std::abs(n);
		f = std::abs(f);
		if (n>f) 
			std::swap(n, f);
		float l, r, b, t;
		t = n*tan(wyc::deg2rad(fov*0.5f));
		b = -t;
		r = t*aspect;
		l = -r;
		proj = {
			2*n/(r-l),	0,			(r+l)/(r-l),	0,
			0,			2*n/(t-b),	(t+b)/(t-b),	0,
			0,			0,			(f+n)/(n-f),	2*f*n/(n-f),
			0,			0,			-1,				0
		};
	}

	void set_translate(Matrix33f &m, float dx, float dy)
	{
		m.makeIdentity();
		m[0][2] = dx;
		m[1][2] = dy;
	}

	void set_scale(Matrix33f &m, float sx, float sy)
	{
		m.makeIdentity();
		m[0][0] = sx;
		m[1][1] = sy;
	}

	void set_rotate(Matrix33f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[1][1] = std::cos(radian); 
		m[0][1] = -sina;
		m[1][0] =  sina; 
	}

	void set_translate(Matrix44f &m, float dx, float dy, float dz)
	{
		m.makeIdentity();
		m[0][3] = dx;
		m[1][3] = dy;
		m[2][3] = dz;
	}

	void set_scale(Matrix44f &m, float sx, float sy, float sz)
	{
		m.makeIdentity();
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
	}

	void set_rotate(Matrix44f &m, const Vec3f &n, float radian)
	{
		float sina = std::sin(radian);
		float cosa = std::cos(radian);
		float t = 1 - cosa;
		m = {
			n.x*n.x*t + cosa,		n.x*n.y*t - n.z*sina,	n.x*n.z*t + n.y*sina,	0,
			n.x*n.y*t + n.z*sina,	n.y*n.y*t + cosa,		n.y*n.z*t - n.x*sina,	0,
			n.x*n.z*t - n.y*sina,	n.z*n.y*t + n.x*sina,	n.z*n.z*t + cosa,		0,
			0,						0,						0,						1
		};
	}

	void set_rotate_x(Matrix44f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[1][1] = m[2][2] = std::cos(radian);
		m[1][2] = -sina;
		m[2][1] =  sina;
	}

	void set_rotate_y(Matrix44f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[2][2] = std::cos(radian);
		m[0][2] =  sina;
		m[2][0] = -sina;
	}

	void set_rotate_z(Matrix44f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[1][1] = std::cos(radian);
		m[0][1] = -sina;
		m[1][0] =  sina;
	}

	void clip_polygon_by_plane(const Vec4f &plane, const std::vector<Vec3f> &vertices, std::vector<Vec3f> &out)
	{
		Vec3f prev = vertices.back();
		float pdot = prev ^ plane;
		out.reserve(vertices.size() + 1);
		for (auto &vert : vertices)
		{
			float dot = vert ^ plane;
			if (dot * pdot < 0)
				out.push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
	}

	void clip_polygon(const std::vector<Vec4f> &planes, std::vector<Vec3f> &vertices)
	{
		for (auto plane : planes)
		{
			std::vector<Vec3f> tmp;
			clip_polygon_by_plane(plane, vertices, tmp);
			vertices = std::move(tmp);
			if (vertices.empty())
				return;
		}
	}

	void _clip_comp(std::vector<Vec4f> *vertices, std::vector<Vec4f> *out, int i)
	{
		Vec4f prev = vertices->back();
		float pdot = prev.w - prev[i], dot;
		for (auto &vert : *vertices)
		{
			dot = vert.w - vert[i];
			if (pdot * dot < 0)
				out->push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out->push_back(vert);
			prev = vert;
			pdot = dot;
		}
	}

	void _clip_comp_neg(std::vector<Vec4f> *vertices, std::vector<Vec4f> *out, int i)
	{
		Vec4f prev = vertices->back();
		float pdot = prev.w + prev[i], dot;
		for (auto &vert : *vertices)
		{
			dot = vert.w + vert[i];
			if (pdot * dot < 0)
				out->push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out->push_back(vert);
			prev = vert;
			pdot = dot;
		}
	}

	void clip_polygon_homo(std::vector<Vec4f> &vertices)
	{
		std::vector<Vec4f> out;
		// clipped by 7 planes may result 7 more vertices at most
		out.reserve(vertices.size() + 7);
		float pdot, dot;
		Vec4f prev;
		// clipped by W=0
		const float w_epsilon = 0.0001f;
		prev = vertices.back();
		pdot = prev.w - w_epsilon;
		for (auto &vert : vertices)
		{
			dot = vert.w - w_epsilon;
			if (pdot * dot < 0)
				out.push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by positive plane: W=X, W=Y, W=Z
		for (int i = 0; i < 3; ++i)
		{
			_clip_comp(&vertices, &out, i);
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			_clip_comp_neg(&vertices, &out, i);
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
	}

	void intersect(float *v1, float *v2, float d1, float d2, size_t component, float *out)
	{
		float t = d1 / (d1 - d2);
		if (d1 < 0)
			t = fast_ceil(t * 1000) * 0.001f;
		else
			t = fast_floor(t * 1000) * 0.001f;
		//return p1 + (p2 - p1) * t;
		for (float *end = out + component; out != end; ++out, ++v1, ++v2)
		{
			*out = (*v1) + (*v2 - *v1) * t;
		}
	}

	size_t clip_polygon(float *vertex_in, float *vertex_out, size_t stride, size_t pos_offset, size_t size_in, size_t max_size_out)
	{
		using vec4_t = Imath::V4f;
		float pdot, dot;
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		size_t prev_i = stride * (size_in - 1);
		float *ptr_pos = vertex_in + prev_i + pos_offset;
		vec4_t *pos = (vec4_t*)(ptr_pos);
		pdot = pos->w - w_epsilon;
		size_t size_out = 0;
		for (size_t i = 0, end = size_in * stride; i < end; i += stride)
		{
			//const auto &pos = in[i].pos;
			ptr_pos += stride;
			pos = (vec4_t*)(ptr_pos);
			dot = pos->w - w_epsilon;
			if (pdot * dot < 0) {
				assert(size_out < max_size_out && "vertex cache overflow");
				//out[out_size++] = intersect(in[prev_idx], pdot, in[i], dot);
				intersect(vertex_in + prev_i, vertex_in + i, pdot, dot, stride, vertex_out + size_out);
				size_out += stride;
			}
			if (dot >= 0) {
				assert(size_out < max_size_out && "vertex cache overflow");
				//out[out_size++] = in[i];
				memcpy(vertex_out + size_out, vertex_in + i, sizeof(float) * stride);
				size_out += stride;
			}
			prev_i = i;
			pdot = dot;
		}
		if (!size_out)
			return 0;
		std::swap(vertex_in, vertex_out);
		size_in = size_out;
		size_out = 0;

		//// clipped by positive plane: W=X, W=Y, W=Z
		//for (size_t i = 0; i < 3; ++i)
		//{
		//	prev_idx = size - 1;
		//	const auto &pos = in[prev_idx].pos;
		//	float pdot = pos.w - pos[i], dot;
		//	for (size_t k = 0; k < size; ++k)
		//	{
		//		const auto &pos = in[k].pos;
		//		dot = pos.w - pos[i];
		//		if (pdot * dot < 0) {
		//			assert(out_size < max_size && "vertex cache overflow");
		//			out[out_size++] = intersect(in[prev_idx], pdot, in[k], dot);
		//		}
		//		if (dot >= 0) {
		//			assert(out_size < max_size && "vertex cache overflow");
		//			out[out_size++] = in[k];
		//		}
		//		prev_idx = i;
		//		pdot = dot;
		//	}
		//	if (!out_size)
		//		return nullptr;
		//	std::swap(in, out);
		//	size = out_size;
		//	out_size = 0;
		//}
		//// clipped by negative plane: W=-X, W=-Y, W=-Z
		//for (size_t i = 0; i < 3; ++i)
		//{
		//	prev_idx = size - 1;
		//	const auto &pos = in[prev_idx].pos;
		//	float pdot = pos.w + pos[i], dot;
		//	for (size_t k = 0; k < size; ++k)
		//	{
		//		const auto &pos = in[k].pos;
		//		dot = pos.w + pos[i];
		//		if (pdot * dot < 0)
		//		{
		//			assert(out_size < max_size && "vertex cache overflow");
		//			out[out_size++] = intersect(in[prev_idx], pdot, in[k], dot);
		//		}
		//		if (dot >= 0)
		//		{
		//			assert(out_size < max_size && "vertex cache overflow");
		//			out[out_size++] = in[k];
		//		}
		//		prev_idx = i;
		//		pdot = dot;
		//	}
		//	if (!out_size)
		//		return nullptr;
		//	std::swap(in, out);
		//	size = out_size;
		//	out_size = 0;
		//}
		return size_out;
	}

} // namespace wyc

