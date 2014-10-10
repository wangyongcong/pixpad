#include "vecmath.h"

namespace wyc
{
	void set_orthograph(mat4f_t &proj, float l, float b, float n, float r, float t, float f)
	{
		proj = {
			2.0f/(r-l),	0,			0,			(r+l)/(l-r),
			0,			2.0f/(t-b),	0,			(t+b)/(b-t),
			0,			0,			2.0f/(n-f), (f+n)/(n-f),
			0,			0,			0,			1
		};
	}

	void set_perspective(mat4f_t &proj, float fov, float aspect, float n, float f)
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

	void clip_polygons(const std::vector<vec4f_t> &planes, std::vector<vec3f_t> &vertices)
	{
		for (const auto &plane : planes)
		{
			float pdot = 0;
			vec3f_t prev;
			for (auto &vert : vertices)
			{
				float dot = vert ^ plane;
				if (dot * pdot < 0)
				{
					float t = pdot / (pdot - dot);
					vec3f_t intersect = prev + (vert - prev) * t;
					prev = vert;
					vert = intersect;
				}
				else
				{
					prev = vert;
				}
				pdot = dot;
			}
		}
	}

} // namespace wyc

