#include "raster.h"
#include <cassert>
#include <OpenEXR/ImathMath.h>
#include "math/vector.h"

namespace wyc
{
	inline bool fequal(float v1, float v2)
	{
		return 	IMATH_NAMESPACE::equalWithAbsError(v1, v2, std::numeric_limits<float>::epsilon());
	}

	void xsurface_test::detail() const
	{
		int cur_line = -1;
		std::vector<fragment>::const_reverse_iterator iter, end;
		for (iter = m_frags.rbegin(), end = m_frags.rend(); iter != end; ++iter)
		{
			if (cur_line != iter->y)
			{
				// new line
				cur_line = iter->y;
				printf("\nLine %02d\t%02d", cur_line, iter->x);
			}
			else
				printf(", %02d", iter->x);
		}
		printf("\n");
	}

	void xsurface_test::verify(const vec2f &center, const vec2f verts[3]) const
	{
		const float rel_err = 0.0005f;
		const vec2f *p0, *p1, *p2;
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
		float dx10 = p1->x - p0->x;
		float dy10 = p1->y - p0->y;
		float dx20 = p2->x - p0->x;
		float dy20 = p2->y - p0->y;
		float dx21 = p2->x - p1->x;
		float dy21 = p2->y - p1->y;
		float real_x, real_y, x0, x1;
		for (size_t i = 0, cnt = m_frags.size(); i < cnt;)
		{
			const fragment &iter = m_frags[i];
			int cur_y = iter.y;
			real_y = iter.y + center.y;
			assert(real_y >= p0->y && real_y < p2->y);
			if (real_y < p1->y)
			{
				float dy = real_y - p0->y;
				x0 = p0->x + dy * dx10 / dy10;
				x1 = p0->x + dy * dx20 / dy20;
			}
			else
			{
				x0 = p0->x + (real_y - p0->y) * dx20 / dy20;
				x1 = p1->x + (real_y - p1->y) * dx21 / dy21;
			}
			if (x0 > x1)
				std::swap(x0, x1);
			// make sure the previous sample is out of range
			real_x = iter.x + center.x - 1;
			assert(real_x < x0 || std::fabs(real_x - x0) < rel_err);
			// make sure the sample is within the triangle
			do {
				real_x = m_frags[i].x + center.x;
				assert(real_x > x0 || std::fabs(real_x - x0) < rel_err);
				assert(real_x < x1 || std::fabs(real_x - x1) < rel_err);
				i += 1;
			} while (i < cnt && m_frags[i].y == cur_y);
			// make sure the next sample is out of range
			real_x += 1;
			assert(real_x > x1 || std::fabs(real_x - x1) < rel_err);
		}
	}

	void sample_scan_line(float y, float cx, float lx, float rx, xsurface_test &surf)
	{
		if (fequal(lx, rx))
			return;
		assert(lx <= rx);
		float x, t, dt;
		x = std::floor(lx + 1 - cx) + cx;
		dt = 1.0f / (rx - lx);
		t = (x - lx) * dt;
		// NOTICE: not include the right border
		for (; x < rx; x += 1)
		{
			t += dt;
			surf.plot(int(x), int(y), t);
		}
	}

	void sample_triangle(const vec2f &center, const vec2f verts[3], xsurface_test &surf)
	{
		const vec2f *p0, *p1, *p2;
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
		float x0, x1, t0, t1, dx0, dx1, dt0, dt1;
		float dy10 = p1->y - p0->y;
		float dy20 = p2->y - p0->y;
		float dy21 = p2->y - p1->y;
		float y = std::floor(p0->y + 1 - center.y) + center.y;
		float dy = y - p0->y;
		bool p1_first;
		if (!fequal(dy10, 0))
		{
			assert(!fequal(dy20, 0));
			dt0 = 1.0f / dy20;
			float dx20 = p2->x - p0->x;
			p1_first = p1->x <= (p0->x + dx20 * dt0 * dy10);
			if (p1_first)
			{
				dt1 = dt0;
				dt0 = 1.0f / dy10;
				dx0 = (p1->x - p0->x) * dt0;
				dx1 = dx20 * dt1;
			}
			else
			{
				dt1 = 1.0f / dy10;
				dx0 = dx20 * dt0;
				dx1 = (p1->x - p0->x) * dt1;
			}
			x0 = p0->x + dy * dx0;
			x1 = p0->x + dy * dx1;
			t0 = dy * dt0;
			t1 = dy * dt1;
			for (; y < p1->y; y += 1)
			{
				sample_scan_line(y, center.x, x0, x1, surf);
				x0 += dx0;
				x1 += dx1;
				t0 += dt0;
				t1 += dt1;
			}
		}
		else
		{
			p1_first = p1->x < p0->x;
		}
		if (!fequal(dy21, 0))
		{
			assert(!fequal(dy20, 0));
			dy = y - p1->y;
			if (p1_first)
			{
				dt0 = 1.0f / dy21;
				dx0 = (p2->x - p1->x) * dt0;
				x0 = p1->x + dy * dx0;
				t0 = dy * dt0;
			}
			else
			{
				dt1 = 1.0f / dy21;
				dx1 = (p2->x - p1->x) * dt1;
				x1 = p1->x + dy * dx1;
				t1 = dy * dt1;
			}
			// NOTICE: include the upper scan line
			for (; y <= p2->y; y += 1)
			{
				sample_scan_line(y, center.x, x0, x1, surf);
				x0 += dx0;
				x1 += dx1;
				t0 += dt0;
				t1 += dt1;
			}
		}
	}

} // end of namespace wyc
