#pragma once
#include <algorithm>
#include "engine.h"
#include "mathex/vecmath.h"

namespace wyc
{
	template<class Vertex>
	int mipmap_level(const Vertex &duv_dx, const Vertex &duv_dy, unsigned texture_size, float &f) {
		float r1 = std::sqrtf(duv_dx ^ duv_dx);
		float r2 = std::sqrtf(duv_dy ^ duv_dy);
		union {
			float f;
			unsigned i;
		} j;
		j.f = std::max(r1, r2) * texture_size;
		int e = (j.i & 0x7f800000) >> 23;
		if (e > 127) {
			e -= 127;
			f = float(j.i & 0x7fffff) / (1 << 23);
			return e;
		}
		return 0;
	}

	WYCAPI int mipmap_level(float dudx, float dudy, unsigned texture_size, float &f);

	WYCAPI float smooth_step(float edge0, float edge1, float x);

	template<class Vector>
	inline Vector safe_divide(const Vector &v1, const Vector &v2) {
		return v1 / v2;
	}

	template<>
	inline vec3f safe_divide<vec3f>(const vec3f &v1, const vec3f &v2) {
		return{ 
			v2.x != 0 ? v1.x / v2.x : 0, 
			v2.y != 0 ? v1.y / v2.y : 0,
			v2.z != 0 ? v1.z / v2.z : 0,
		};
	}

	template<>
	inline vec4f safe_divide<vec4f>(const vec4f &v1, const vec4f &v2) {
		return{
			v2.x != 0 ? v1.x / v2.x : 0,
			v2.y != 0 ? v1.y / v2.y : 0,
			v2.z != 0 ? v1.z / v2.z : 0,
			v2.w != 0 ? v1.w / v2.w : 0,
		};
	}

	template<class Vertex>
	Vertex smooth_step(const Vertex &edge0, const Vertex &edge1, float x)
	{
		Vertex vx = Vertex(x);
		Vertex d1 = vx - edge0;
		Vertex d2 = edge1 - edge0;
		Vertex t = safe_divide(d1, d2);
		return t * t * (Vertex(3.0f) - 2.0f * t);
	}

} // namespace wyc
