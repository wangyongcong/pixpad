#include "ImathMatrix.h"
#include "shader_api.h"

namespace wyc
{
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

} // namespace wyc
