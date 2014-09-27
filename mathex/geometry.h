#ifndef __HEADER_WYC_XGEOMETRY
#define __HEADER_WYC_XGEOMETRY


#include <algorithm>
#include "vecmath.h"

namespace wyc
{

	// ray = org + t * dir
	// aabb = [lower, upper]
	// detect ray-aabb intersection
	// return true if hit, and the intersection point is given by
	// p = org + t * dir
	bool intersect_ray_aabb(const xvec2f_t &org, const xvec2f_t &dir, const xvec2f_t &lower, const xvec2f_t &upper, float &t)
	{
		float t1, t2, t3, t4, inv_x, inv_y;

		if (fabs(dir.y) < EPSILON_E4) // horizontal line
		{
			inv_x = dir.x > 0 ? 1.0f : -1.0f;
			t1 = (lower.x - org.x)*inv_x;
			t2 = (upper.x - org.x)*inv_x;
			t = std::min(t1, t2);
			return org.y >= lower.y && org.y < upper.y && (t1>0 || t2 > 0);
		}

		if (fabs(dir.x) < EPSILON_E4) // vertical line
		{
			inv_y = dir.y > 0 ? 1.0f : -1.0f;
			t3 = (lower.y - org.y)*inv_y;
			t4 = (upper.y - org.y)*inv_y;
			t = std::min(t3, t4);
			return org.x >= lower.x && org.x < upper.x && (t3>0 || t4 > 0);
		}

		inv_x = 1.0f / dir.x, inv_y = 1.0f / dir.y;
		t1 = (lower.x - org.x)*inv_x;
		t2 = (upper.x - org.x)*inv_x;
		t3 = (lower.y - org.y)*inv_y;
		t4 = (upper.y - org.y)*inv_y;

		float tmin = std::max(std::min(t1, t2), std::min(t3, t4)); // entry point
		float tmax = std::min(std::max(t1, t2), std::max(t3, t4)); // leave point

		if (tmin < tmax)
		{
			t = tmin;
			return true;
		}
		t = tmax;
		return false;
	}

} // namespace wyc

#endif // end of __HEADER_WYC_XGEOMETRY
