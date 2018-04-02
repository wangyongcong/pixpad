#include "vecmath.h"
#include <cassert>
#include <ImathMatrix.h>

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
		m.makeIdentity();
		m[0][2] = dx;
		m[1][2] = dy;
	}

	void set_scale(mat3f &m, float sx, float sy)
	{
		m.makeIdentity();
		m[0][0] = sx;
		m[1][1] = sy;
	}

	void set_rotate(mat3f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[1][1] = std::cos(radian); 
		m[0][1] = -sina;
		m[1][0] =  sina; 
	}

	void set_translate(mat4f &m, float dx, float dy, float dz)
	{
		m.makeIdentity();
		m[0][3] = dx;
		m[1][3] = dy;
		m[2][3] = dz;
	}

	void set_scale(mat4f &m, float sx, float sy, float sz)
	{
		m.makeIdentity();
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
		m.makeIdentity();
		m[1][1] = m[2][2] = std::cos(radian);
		m[1][2] = -sina;
		m[2][1] =  sina;
	}

	void set_rotate_y(mat4f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[2][2] = std::cos(radian);
		m[0][2] =  sina;
		m[2][0] = -sina;
	}

	void set_rotate_z(mat4f &m, float radian)
	{
		float sina = std::sin(radian);
		m.makeIdentity();
		m[0][0] = m[1][1] = std::cos(radian);
		m[0][1] = -sina;
		m[1][0] =  sina;
	}

} // namespace wyc

