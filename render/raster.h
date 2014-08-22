#ifndef WYC_HEADER_RASTER
#define WYC_HEADER_RASTER

#include <algorithm>
#include <vector>
#include <float.h>
#include "mathex/vecmath.h"

namespace wyc
{

class xsurface
{
public:
	xsurface() {
		m_minx = INT_MAX;
		m_maxx = INT_MIN;
	}
	xsurface(const xsurface&) = delete;
	xsurface& operator = (const xsurface&) = delete;
	void clear() {
		m_frags.clear();
	}
	void plot(int x, int y, float t)
	{
		fragment v = {x, y, t};
		m_frags.push_back(v);
		if(x < m_minx)
			m_minx = x;
		if(x > m_maxx)
			m_maxx = x;
	}
	void detail() const;
	void verify(const xvec2f_t &center, const xvec2f_t verts[3]) const;

private:
	struct fragment {
		int x;
		int y;
		float t;
	};

	std::vector<fragment> m_frags;
	int m_minx, m_maxx;
};

void sample_scan_line(float y, float cx, float x0, float x1, xsurface &surf);

void sample_triangle(const xvec2f_t &center, const xvec2f_t verts[3], xsurface &surf);

} // end of namespace wyc

#endif // WYC_HEADER_RASTER
