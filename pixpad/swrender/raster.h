#ifndef WYC_HEADER_RASTER
#define WYC_HEADER_RASTER
#include <algorithm>
#include "mathex/vecmath.h"

namespace wyc
{
	template<typename sampler_t, typename vertex_t>
	void sample_triangle(sampler_t sampler, const vertex_t verts[3])
	{
		vertex_t *p0, p1, p2;
		if (verts[0].y > verts[1].y) {
			p0 = verts + 1;
			p1 = verts;
		}
		else {
			p0 = verts;
			p1 = verts + 1;
		}
		if (p0->y > verts[2].y) {
			p2 = p0;
			p0 = verts + 2;
		}
		else
			p2 = verts + 2;
		if (p1->y > p2->y)
			std::swap(p1, p2);
		assert(p0.y <= p1->y <= p2.y);
	}

}

#endif // WYC_HEADER_RASTER
