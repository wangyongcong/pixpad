#pragma once

#include "mathfwd.h"

namespace wyc
{
	// bresenham line rasterization
	template<typename Plotter>
	void line_bresenham(Plotter &plot, Vec2f &v0, const Vec2f &v1)
	{
		int x0, y0, x1, y1;
		x0 = fast_round(v0.x);
		y0 = fast_round(v0.y);
		x1 = fast_round(v1.x);
		y1 = fast_round(v1.y);
		if (x0>x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int dx = x1 - x0;
		int dy = y1 - y0;
		int advance;
		if (dy < 0)
		{
			dy = -dy;
			advance = -1;
		}
		else
			advance = 1;
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
					plot.skip_row(advance);
					d -= dx << 1;
				}
				plot();
			}
		}
		else
		{
			int m = dx << 1;
			int d = -dy;
			for (int i = 0; i < dy; ++i)
			{
				plot.skip_row(advance);
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

	// calculate the signed-area * 2 of triangle (v0, v1, v2)
	// return positive value if counter-clockwise, 
	// negative value for clockwise, 
	// 0 for degenerated triangle (v0, v1, v2 are collinear)
	template<typename T>
	inline T triangle_edge_function(const Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
	{
		return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
	}

	template<typename T>
	bool inside_triangle(const Imath::Vec2 &p, Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
	{
		T edge01 = triangle_edge_function(v0, v1, p);
		T edge12 = triangle_edge_function(v1, v2, p);
		T edge20 = triangle_edge_function(v2, v0, p);
		return edge01 >= 0 && edge12 >= 0 && edge20 >= 0;
	}

	template<typename T>
	Imath::Vec3<T> barycentric_coord(const Imath::Vec2 &p, Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
	{
		Imath::Vec3<T> bc;
		bc.x = triangle_edge_function(v1, v2, p);
		bc.y = triangle_edge_function(v2, v0, p);
		bc.z = triangle_edge_function(v0, v1, p);
		return bc;
	}

	template<typename T>
	void normalize_barycentric_coord(Imath::Vec3<T> &bc)
	{
		T area = bc.x + bc.y + bc.z;
		if (area != T(0))
			bc /= area;
	}

} // namespace wyc
