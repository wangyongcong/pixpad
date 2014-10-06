#ifndef WYC_HEADER_RASTER
#define WYC_HEADER_RASTER

#include <algorithm>
#include <vector>
#include <float.h>
#include "surface.h"
#include "vecmath.h"

namespace wyc
{

	class xsurface_test
	{
	public:
		xsurface_test() {
			m_minx = INT_MAX;
			m_maxx = INT_MIN;
		}
		xsurface_test(const xsurface_test&) = delete;
		xsurface_test& operator = (const xsurface_test&) = delete;
		void clear() {
			m_frags.clear();
		}
		void plot(int x, int y, float t)
		{
			fragment v = { x, y, t };
			m_frags.push_back(v);
			if (x < m_minx)
				m_minx = x;
			if (x > m_maxx)
				m_maxx = x;
		}
		void detail() const;
		void verify(const vec2f_t &center, const vec2f_t verts[3]) const;

	private:
		struct fragment {
			int x;
			int y;
			float t;
		};

		std::vector<fragment> m_frags;
		int m_minx, m_maxx;
	};

	void sample_scan_line(float y, float cx, float x0, float x1, xsurface_test &surf);

	void sample_triangle(const vec2f_t &center, const vec2f_t verts[3], xsurface_test &surf);

	template<typename T>
	class xplotter
	{
	public:
		xplotter(xsurface &surf, T v) : m_surf(surf), m_val(v)
		{
			m_pitch = surf.pitch();
		}
		inline void operator() (int x, int y)
		{
			m_cur = m_surf.get<T>(x, y);
			assert(m_surf.validate(m_cur));
			*m_cur = m_val;
		}
		inline void operator() () {
			assert(m_surf.validate(m_cur));
			*m_cur = m_val;
		}
		inline void operator += (int v) {
			m_cur += v;
		}
		inline void advance(int v) {
			m_cur = (T*)(((uint8_t*)m_cur) + v * m_pitch);
		}

	private:
		xsurface &m_surf;
		size_t m_pitch;
		T *m_cur;
		T m_val;
	};

	template<typename T>
	void line_sampler(unsigned x0, unsigned y0, unsigned x1, unsigned y1, T &plot)
	{
		if (x0>x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int dx = x1 - x0;
		int dy = y1 - y0;
		int pitch;
		if (dy<0)
		{
			dy = -dy;
			pitch = -1;
		}
		else 
			pitch = 1;
		plot(x0, y0);
		if (dx >= dy)
		{
			int m = dy << 1;
			int d = -dx;
			for (int i = 0; i<dx; ++i)
			{
				plot += 1;
				d += m;
				if (d >= 0)
				{
					plot.advance(pitch);
					d -= dx << 1;
				}
				plot();
			}
		}
		else
		{
			int m = dx << 1;
			int d = -dy;
			for (int i = 0; i<dy; ++i)
			{
				plot.advance(pitch);
				d += m;
				if (d >= 0)
				{
					plot += 1;
					d -= dy << 1;
				}
				plot();
			}
		}

	}

} // end of namespace wyc

#endif // WYC_HEADER_RASTER
