#pragma once
#include "OpenEXR/ImathExc.h"
#include "OpenEXR/ImathBox.h"
#include "mathfwd.h"
#include "floatmath.h"

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
	bool is_inside_triangle(const Imath::Vec2 &p, Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
	{
		T edge01 = triangle_edge_function(v0, v1, p);
		T edge12 = triangle_edge_function(v1, v2, p);
		T edge20 = triangle_edge_function(v2, v0, p);
		return edge01 >= 0 && edge12 >= 0 && edge20 >= 0;
	}

	template<typename T>
	Imath::Vec3<T> barycentric_coord(const Imath::Vec2 &p, Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
	{
		T x = triangle_edge_function(v1, v2, p);
		T y = triangle_edge_function(v2, v0, p);
		T z = triangle_edge_function(v0, v1, p);
		T area = x + y + z;
#ifdef _DEBUG
		if (area == T(0))
			throw Imath::NullVecExc("Null barycentric vector.");
#endif			
		return Imath::Vec3<T>(x / area, y / area, z / area);
	}

	bool is_top_left(const Vec2f &v1, const Vec2f &v2)
	{
		return false;
	}

	template<uint16 N>
	inline Vec2i snap_to_subpixel(const Vec2f &v)
	{
		return Vec2i(fast_to_fixed<N>(v.x), fast_to_fixed<N>(v.y));
	}

	// fill triangle {vertices[0], vertices[1], vertices[2]} in counter-clockwise
	template<typename Plotter>
	void fill_triangle(const Imath::Box<Vec2i> &box, const Vec2f *vertices, Plotter &plot)
	{
#ifdef _DEBUG
		// max render target is 2048 x 2048
		assert(box.min.x >= -1024 && box.min.x < 1024 
			&& box.min.y >= -1024 && box.min.y < 1024
			&& box.max.x >= -1024 && box.max.x < 1024
			&& box.max.y >= -1024 && box.max.y < 1024);
#endif
		
		// snap to .8 sub pixel
		Vec2i v0 = snap_to_subpixel<8>(vertices[0]);
		Vec2i v1 = snap_to_subpixel<8>(vertices[1]);
		Vec2i v2 = snap_to_subpixel<8>(vertices[2]);
		
		// edge function delta
		int32 edge_a01 = v0.y - v1.y, edge_b01 = v1.x - v0.x;
		int32 edge_a12 = v1.y - v2.y, edge_b12 = v2.x - v1.x;
		int32 edge_a20 = v2.y - v0.y, edge_b20 = v0.x - v2.x;

		// initial edge function
		int32 row_w0 = triangle_edge_function(v1, v2, box.min);
		int32 row_w1 = triangle_edge_function(v2, v0, box.min);
		int32 row_w2 = triangle_edge_function(v0, v1, box.min);

		int32 w0, w1, w2;
		const int32 step = 256;
		for (int32 y = box.min.y; y < box.max.y; y += step)
		{
			w0 = row_w0;
			w1 = row_w1;
			w2 = row_w2;
			for (int32 x = box.min.x; x < box.max.x; x += step)
			{
				if ((w0 | w1 | w2) > 0)
					plot(x, y);
				w0 += edge_a12;
				w1 += edge_a20;
				w2 += edge_a01;
			}
			row_w0 += edge_b12;
			row_w1 += edge_b20;
			row_w2 += edge_b01;
		}
	}



} // namespace wyc
