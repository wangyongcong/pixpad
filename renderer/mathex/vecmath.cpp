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

	void clip_polygon_homo(std::vector<Vec4f> &vertices)
	{
		std::vector<Vec4f> out;
		// clipped by 7 planes may result 7 more vertices at most
		out.reserve(vertices.size() + 7);
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		float pdot, dot;
		Vec4f prev = vertices.back();
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
			prev = vertices.back();
			pdot = prev.w - prev[i], dot;
			for (auto &vert : vertices)
			{
				dot = vert.w - vert[i];
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
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			prev = vertices.back();
			pdot = prev.w + prev[i], dot;
			for (auto &vert : vertices)
			{
				dot = vert.w + vert[i];
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
		}
	}

	void intersect(float *v1, float d1, float *v2, float d2, size_t component, float *out)
	{
		float t = d1 / (d1 - d2);
		if (d1 < 0)
			t = fast_ceil(t * 1000) * 0.001f;
		else
			t = fast_floor(t * 1000) * 0.001f;
		// v1 + (v2 - v1) * t;
		for (float *end = out + component; out != end; ++out, ++v1, ++v2)
		{
			*out = (*v1) + (*v2 - *v1) * t;
		}
	}

	float* clip_polygon_stream(float *vertex_in, float *vertex_out, size_t &vertex_count, size_t stride, size_t pos_offset, size_t cache_size)
	{
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		float *prev_vert = vertex_in + stride * (vertex_count - 1);
		float *cur_vert = vertex_in;
		float *out_vert = vertex_out;
		float *end = cur_vert + (vertex_count * stride);
		// pos = {x, y, z, w}
		float *pos = prev_vert + pos_offset;
		float pdot, dot;
		pdot = pos[3] - w_epsilon;
		vertex_count = 0;
		for (; cur_vert < end; cur_vert += stride)
		{
			pos = cur_vert + pos_offset;
			dot = pos[3] - w_epsilon;
			if (pdot * dot < 0) {
				assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
				intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
				out_vert += stride;
				vertex_count += 1;
			}
			if (dot >= 0) {
				assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
				memcpy(out_vert, cur_vert, sizeof(float) * stride);
				out_vert += stride;
				vertex_count += 1;
			}
			prev_vert = cur_vert;
			pdot = dot;
		}
		if (!vertex_count)
			return nullptr;
		// clipped by positive plane: W=X, W=Y, W=Z
		for (int i = 0; i < 3; ++i)
		{
			std::swap(vertex_in, vertex_out);
			prev_vert = out_vert - stride;
			end = out_vert;
			cur_vert = vertex_in;
			out_vert = vertex_out;			
			vertex_count = 0;
			pos = prev_vert + pos_offset;
			pdot = pos[3] - pos[i], dot;
			for (; cur_vert < end; cur_vert += stride)
			{
				pos = cur_vert + pos_offset;
				dot = pos[3] - pos[i];
				if (pdot * dot < 0) {
					assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
					out_vert += stride;
					vertex_count += 1;
				}
				if (dot >= 0) {
					assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					memcpy(out_vert, cur_vert, sizeof(float) * stride);
					out_vert += stride;
					vertex_count += 1;
				}
				prev_vert = cur_vert;
				pdot = dot;
			}
			if (!vertex_count)
				return nullptr;
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			std::swap(vertex_in, vertex_out);
			prev_vert = out_vert - stride;
			end = out_vert;
			cur_vert = vertex_in;
			out_vert = vertex_out;
			vertex_count = 0;
			pos = prev_vert + pos_offset;
			pdot = pos[3] + pos[i], dot;
			for (; cur_vert < end; cur_vert += stride)
			{
				pos = cur_vert + pos_offset;
				dot = pos[3] + pos[i];
				if (pdot * dot < 0)
				{
					assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
					out_vert += stride;
					vertex_count += 1;
				}
				if (dot >= 0)
				{
					assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					memcpy(out_vert, cur_vert, sizeof(float) * stride);
					out_vert += stride;
					vertex_count += 1;
				}
				prev_vert = cur_vert;
				cur_vert += stride;
				pdot = dot;
			}
			if (!vertex_count)
				return nullptr;
		}
		return vertex_out;
	}

} // namespace wyc

