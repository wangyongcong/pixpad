#include <cassert>
#include "vecmath.h"
#include "vector.h"
#include "matrix.h"

namespace wyc
{
	void set_orthograph(mat4f &proj, float l, float b, float n, float r, float t, float f)
	{
		proj = {
			2/(r-l),	0,			0,			(r+l)/(l-r),
			0,			2/(t-b),	0,			(t+b)/(b-t),
			0,			0,			2/(n-f),	(f+n)/(n-f),
			0,			0,			0,			1
		};
	}

	void set_perspective(mat4f &proj, float fov, float aspect, float n, float f)
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

	void set_translate(mat3f &m, float dx, float dy)
	{
		m.identity();
		m[0][2] = dx;
		m[1][2] = dy;
	}

	void set_scale(mat3f &m, float sx, float sy)
	{
		m.identity();
		m[0][0] = sx;
		m[1][1] = sy;
	}

	void set_rotate(mat3f &m, float radian)
	{
		float sina = std::sin(radian);
		m.identity();
		m[0][0] = m[1][1] = std::cos(radian); 
		m[0][1] = -sina;
		m[1][0] =  sina; 
	}

	void set_translate(mat4f &m, float dx, float dy, float dz)
	{
		m.identity();
		m[0][3] = dx;
		m[1][3] = dy;
		m[2][3] = dz;
	}

	void set_scale(mat4f &m, float sx, float sy, float sz)
	{
		m.identity();
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
	}

	void set_rotate(mat4f &m, const vec3f &n, float radian)
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

	void set_rotate_x(mat4f &m, float radian)
	{
		float sina = std::sin(radian);
		m.identity();
		m[1][1] = m[2][2] = std::cos(radian);
		m[1][2] = -sina;
		m[2][1] =  sina;
	}

	void set_rotate_y(mat4f &m, float radian)
	{
		float sina = std::sin(radian);
		m.identity();
		m[0][0] = m[2][2] = std::cos(radian);
		m[0][2] =  sina;
		m[2][0] = -sina;
	}

	void set_rotate_z(mat4f &m, float radian)
	{
		float sina = std::sin(radian);
		m.identity();
		m[0][0] = m[1][1] = std::cos(radian);
		m[0][1] = -sina;
		m[1][0] =  sina;
	}

	void clip_polygon_by_plane(const vec4f &plane, const std::vector<vec3f> &vertices, std::vector<vec3f> &out)
	{
		vec3f prev = vertices.back();
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

	void clip_polygon(const std::vector<vec4f> &planes, std::vector<vec3f> &vertices)
	{
		for (auto plane : planes)
		{
			std::vector<vec3f> tmp;
			clip_polygon_by_plane(plane, vertices, tmp);
			vertices = std::move(tmp);
			if (vertices.empty())
				return;
		}
	}

	void _clip_comp(std::vector<vec4f> *vertices, std::vector<vec4f> *out, int i)
	{
		vec4f prev = vertices->back();
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

	void _clip_comp_neg(std::vector<vec4f> *vertices, std::vector<vec4f> *out, int i)
	{
		vec4f prev = vertices->back();
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

	void clip_polygon_homo(std::vector<vec4f> &vertices)
	{
		std::vector<vec4f> out;
		// clipped by 6 planes may result 6 more vertices at most
		out.reserve(vertices.size() + 6);
		float pdot, dot;
		vec4f prev;
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

} // namespace wyc

