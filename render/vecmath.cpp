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
			{
				out->push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out->push_back(vert);
			prev = vert;
			pdot = dot;
		}
	}

	void clip_polygon(std::vector<vec4f> &vertices)
	{
		std::vector<vec4f> out;
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
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		//for (int i = 0; i < 3; ++i)
		//{
		//	_clip_comp(&vertices, &out, i);
		//}
		// clipped by W=X
		prev = vertices.back();
		pdot = prev.w - prev.x;
		for (auto &vert : vertices)
		{
			dot = vert.w - vert.x;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by W=-X
		prev = vertices.back();
		pdot = prev.w + prev.x;
		for (auto &vert : vertices)
		{
			dot = vert.w + vert.x;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by W=Y
		prev = vertices.back();
		pdot = prev.w - prev.y;
		for (auto &vert : vertices)
		{
			dot = vert.w - vert.y;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by W=-Y
		prev = vertices.back();
		pdot = prev.w + prev.y;
		for (auto &vert : vertices)
		{
			dot = vert.w + vert.y;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by W=Z
		prev = vertices.back();
		pdot = prev.w - prev.z;
		for (auto &vert : vertices)
		{
			dot = vert.w - vert.z;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by W=-Z
		prev = vertices.back();
		pdot = prev.w + prev.z;
		for (auto &vert : vertices)
		{
			dot = vert.w + vert.z;
			if (pdot * dot < 0)
			{
				out.push_back(intersect(prev, pdot, vert, dot));
			}
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
	}

} // namespace wyc

