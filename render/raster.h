#ifndef WYC_HEADER_RASTER
#define WYC_HEADER_RASTER

#include <algorithm>
#include <vector>
#include "mathex/vecmath.h"

namespace wyc
{

class xsurface
{
public:
	xsurface() {

	}
	xsurface(const xsurface&) = 0;
	xsurface& operator = (const xsurface&) = 0;
	void plot(int x, int y, float t)
	{
		fragment v = {x, y, t};
		m_frags.push_back(v);
	}

private:
	struct fragment {
		int x;
		int y;
		float t;
	};

	std::vector<fragment> m_frags;
};

void sample_scan_line(float y, float cx, float x0, float x1, xsurface &surf);

void sample_triangle(const xvec2f_t &center, const xvec2f_t verts[3], xsurface &surf);

} // end of namespace wyc

#endif // WYC_HEADER_RASTER
