#include "raster.h"

namespace wyc
{

void sample_scan_line(float y, float cx, float lx, float rx, xsurface &surf)
{
	assert(lx <= rx);
	float dx, x, t;
	x = std::floor(lx + 1 - cx) + cx;
	dx = 1.0f / (rx - lx);
	t = (x - lx) * dx;
	for(; x < rx; x += 1)
	{
		t += dx;
		surf.plot(int(x), int(y), t);
	}
}

void sample_triangle(const xvec2f_t &center, const xvec2f_t verts[3], xsurface &surf)
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
	assert(p0->y <= p1->y && p1->y <= p2->y);
	float x0, x1, t0, t1, k0, k1, dt0, dt1;
	float dy10 = p1->y - p0->y;
	float dy20 = p2->y - p0->y;
	float dy21 = p2->y - p1->y;
	float y = std::floor(p0->y + 1 - center.y) + center.y;
	float dy = y - p0->y;
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
		x0 = p0->x + dy * k0;
		x1 = p0->x + dy * k1;
		t0 = dy * dt0;
		t1 = dy * dt1;
		sample_scan_line(y, center.x, x0, x1, surf);
		for (; y < p1->y; y += 1)
		{
			x0 += k0;
			x1 += k1;
			t0 += dt0;
			t1 += dt1;
			sample_scan_line(y, center.x, x0, x1, surf);
		}
		dy = y - p1->y;
	}
	else
	{
		if(p1->x < p0->x)
		{
			x0 = p1->x;
			x1 = p0->x;
		}
		else
		{
			x0 = p0->x;
			x1 = p1->x;
		}
	}
	if(!fequal(dy21, 0))
	{
		if(p1->x < p2->x)
		{
			dt0 = 1.0f / dy21;
			dt1 = 1.0f / dy20;
			k0 = (p2->x - p1->x) * dt0;
			k1 = (p2->x - p0->x) * dt1;
		}
		else
		{
			dt0 = 1.0f / dy20;
			dt1 = 1.0f / dy21;
			k0 = (p2->x - p0->x) * dt0;
			k1 = (p2->x - p1->x) * dt1;
		}
		x0 += dy * k0;
		x1 += dy * k1;
		t0 = dy * dt0;
		t1 = dy * dt1;
		sample_scan_line(y, center.x, x0, x1, surf);
		for(; y < p2->y; y += 1)
		{
			x0 += k0;
			x1 += k1;
			t0 += dt0;
			t1 += dt1;
			sample_scan_line(y, center.x, x0, x1, surf);
		}
	}
}

} // end of namespace wyc
