#pragma once
#include <algorithm>

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
			int m = j.i & 0x7fffff;
			f = float(m) / (1 << 23);
			return m;
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
			int m = j.i & 0x7fffff;
			f = float(m) / (1 << 23);
			return m;
		}
		return 0;
	}


} // namespace wyc
