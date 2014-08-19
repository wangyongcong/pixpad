#include "raster.h"

namespace wyc
{
	extern void sample(...);

	void sample_triangle(const xvec2f_t &center, const xvec2f_t verts[3])
	{
		const xvec2f_t *p0, *p1, *p2;
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
		assert(p0->y <= p1->y <= p2->y);
		float x0, x1;
		float k0, k1;
		float dt0, dt1;
		float dy10 = p1->y - p0->y;
		float dy20 = p2->y - p0->y;
		if (!fequal(dy10, 0))
		{
			assert(!fequal(dy20, 0));
			if (p1->x < p2->x)
			{
				dt0 = 1.0f / dy10;
				dt1 = 1.0f / dy20;
				k0 = (p1->x - p0->x) * dt0;
				k1 = (p2->x - p0->x) * dt1;
			}
			else
			{
				dt0 = 1.0f / dy20;
				dt1 = 1.0f / dy10;
				k0 = (p2->x - p0->x) * dt0;
				k1 = (p1->x - p0->x) * dt1;
			}
			float y = std::floor(p0->y + center.y) + center.y;
			float dy = y - p0->y;
			x0 = p0->x + dy * k0;
			x1 = p0->x + dy * k1;
			float t0 = dy * dt0;
			float t1 = dy * dt1;
			sample(y, x0, x1, t0, t1);
			for (; y < p1->y; y += 1)
			{
				x0 += k0;
				x1 += k1;
				t0 += dt0;
				t1 += dt1;
				sample(y, x0, x1, t0, t1);
			}
			assert(t0 <= 1 && t1 <= 1);
		}

	}

}