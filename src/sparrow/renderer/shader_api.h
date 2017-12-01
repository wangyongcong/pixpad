#pragma once
#include <algorithm>
#include <ImathForward.h>

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

	int mipmap_level(float dudx, float dudy, unsigned texture_size, float &f) {
		union {
			float f;
			unsigned i;
		} j;
		j.f = std::max(std::fabs(dudx), std::fabs(dudy)) * texture_size;
		int e = (j.i & 0x7f800000) >> 23;
		if (e > 127) {
			e -= 127;
			f = float(j.i & 0x7fffff) / (1 << 23);
			return e;
		}
		return 0;
	}

	float smooth_step(float edge0, float edge1, float x) {
		float d = edge1 - edge0;
		if (d == 0)
			return 0;
		float t = (x - edge0) / d;
		return t * t * (3.0f - 2.0f * t);
	}

	template<class Vector>
	inline Vector safe_divide(const Vector &v1, const Vector &v2) {
		return v1 / v2;
	}

	template<>
	inline Imath::V3f safe_divide<Imath::V3f>(const Imath::V3f &v1, const Imath::V3f &v2) {
		return{ 
			v2.x != 0 ? v1.x / v2.x : 0, 
			v2.y != 0 ? v1.y / v2.y : 0,
			v2.z != 0 ? v1.z / v2.z : 0,
		};
	}

	template<>
	inline Imath::V4f safe_divide<Imath::V4f>(const Imath::V4f &v1, const Imath::V4f &v2) {
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
