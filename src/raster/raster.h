#ifndef WYC_HEADER_RASTER
#define WYC_HEADER_RASTER

#include <algorithm>
#include <vector>
#include <float.h>
#include "surface.h"
#include "math/vecmath.h"
#include "math/floatmath.h"
#include "rs_line.h"

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
		void verify(const vec2f &center, const vec2f verts[3]) const;

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

	void sample_triangle(const vec2f &center, const vec2f verts[3], xsurface_test &surf);

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
		inline void operator -= (int v) {
			m_cur -= v;
		}
		inline void skip_row(int v) {
			m_cur = (T*)(((uint8_t*)m_cur) + v * m_pitch);
		}
	private:
		xsurface &m_surf;
		size_t m_pitch;
		T *m_cur;
		T m_val;
	};

	template<typename T>
	inline void draw_line(const vec2f &v0, const vec2f &v1, T &plot)
	{
		int x0, y0, x1, y1;
		x0 = fast_round(v0.x);
		y0 = fast_round(v0.y);
		x1 = fast_round(v1.x);
		y1 = fast_round(v1.y);
		line_bresenham(plot, v0, v1);
	}

	template<typename T>
	inline void draw_line(const vec3f &v0, const vec3f &v1, T &plot)
	{
		draw_line(*(vec2f*)&v0, *(vec2f*)&v1, plot);
	}

	template<typename T>
	inline void draw_line(const vec4f &v0, const vec4f &v1, T &plot)
	{
		draw_line(*(vec2f*)&v0, *(vec2f*)&v1, plot);
	}

	template<typename T>
	void fill_triangle(const vec2f &center, const vec2f verts[3], T &plot)
	{

	}

} // end of namespace wyc

#endif // WYC_HEADER_RASTER