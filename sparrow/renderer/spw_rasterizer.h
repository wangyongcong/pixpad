#pragma once
#include <OpenEXR/ImathExc.h>
#include <OpenEXR/ImathBox.h>
#include "mathfwd.h"
#include "floatmath.h"

namespace wyc
{
	// bresenham line rasterization
	template<typename Vector, typename Plotter>
	void line_bresenham(Plotter &plot, const Vector &v0, const Vector &v1)
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
				x0 += 1;
				d += m;
				if (d >= 0)
				{
					y0 += advance;
					d -= dx << 1;
				}
				plot(x0, y0);
			}
		}
		else
		{
			int m = dx << 1;
			int d = -dy;
			for (int i = 0; i < dy; ++i)
			{
				y0 += advance;
				d += m;
				if (d >= 0)
				{
					x0 += 1;
					d -= dy << 1;
				}
				plot(x0, y0);
			}
		}
	}

	template<typename Vector, typename Plotter>
	void draw_triangle_frame(const Vector &pos0, const Vector &pos1, const Vector &pos2, Plotter &plot)
	{
		line_bresenham(plot, pos0, pos1);
		line_bresenham(plot, pos1, pos2);
		line_bresenham(plot, pos2, pos0);
	}

	// calculate the signed-area * 2 of triangle (v0, v1, v2)
	// return positive value if counter-clockwise, 
	// negative value for clockwise, 
	// 0 for degenerated triangle (v0, v1, v2 are collinear)
	template<typename T, typename Position>
	inline T triangle_edge_function(const Position &v0, const Position &v1, const Position &v2)
	{
		return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
	}

	template<typename T, typename Position>
	bool is_inside_triangle(const Position &p, const Position &v0, const Position &v1, const Position &v2)
	{
		T edge01 = triangle_edge_function(v0, v1, p);
		T edge12 = triangle_edge_function(v1, v2, p);
		T edge20 = triangle_edge_function(v2, v0, p);
		return edge01 >= 0 && edge12 >= 0 && edge20 >= 0;
	}

	template<typename T>
	Imath::Vec3<T> barycentric_coord(const Imath::Vec2<T> &p, Imath::Vec2<T> &v0, const Imath::Vec2<T> &v1, const Imath::Vec2<T> &v2)
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

	template<typename Position>
	inline bool is_top_left(const Position &v1, const Position &v2)
	{
		// In a counter-clockwise triangle:
		// A top edge is an edge that is exactly horizontal and goes towards the left, i.e.its end point is left of its start point.
		// A left edge is an edge that goes down, i.e. its end point is strictly below its start point.
		return (v1.y == v2.y && v1.x > v2.x) || (v1.y > v2.y);
	}

	template<uint16 N, typename Position>
	inline Vec2i snap_to_subpixel(const Position &v)
	{
		return Vec2i(fast_to_fixed<N>(v.x), fast_to_fixed<N>(v.y));
	}

	inline int64_t edge_function_fixed(const Vec2i &v0, const Vec2i &v1, const Vec2i &v2)
	{
		return int64_t(v1.x - v0.x) * int64_t(v2.y - v0.y) - int64_t(v1.y - v0.y) * int64_t(v2.x - v0.x);
	}

	template<typename Vertex>
	inline Vertex interpolate(const Vertex &v0, const Vertex &v1, const Vertex &v2, float t0, float t1, float t2)
	{
		return v0 * t0 + v1 * t1 + v2 * t2;
	}



#ifdef _DEBUG
	#define ASSERT_INSIDE(v, r) assert(v.x >= -r && v.x < r && v.y >= -r && v.y < r)
#else
	#define ASSERT_INSIDE(v, r)
#endif

	// fill triangle {pos0, pos1, pos2} in counter-clockwise
	// position vector: should be 3D vector {x, y, z}
	// plot: functor with following declaration 
	//   void plot(int x, int y, float z, t0, t1, t2)
	template<typename Vector, typename Plotter>
	void fill_triangle(const Imath::Box<Vec2i> &block, const Vector &pos0, const Vector &pos1, const Vector &pos2, Plotter &plot)
	{
		// 11.8 sub pixel precision
		// max render target is 2048 x 2048
		// coordinate must be inside [-1024, 1024)
		ASSERT_INSIDE(block.min, 1024);
		ASSERT_INSIDE(block.max, 1024);
		ASSERT_INSIDE(pos0, 1024);
		ASSERT_INSIDE(pos1, 1024);
		ASSERT_INSIDE(pos2, 1024);
		
		// snap to .8 sub pixel
		Vec2i v0 = snap_to_subpixel<8>(pos0);
		Vec2i v1 = snap_to_subpixel<8>(pos1);
		Vec2i v2 = snap_to_subpixel<8>(pos2);

		// initial edge function with high precision
		// sample point is at pixel center
		Vec2i p = block.min;
		p.x = (p.x << 8) | 0x80;
		p.y = (p.y << 8) | 0x80;
		int64_t hp_w0 = edge_function_fixed(v1, v2, p);
		int64_t hp_w1 = edge_function_fixed(v2, v0, p);
		int64_t hp_w2 = edge_function_fixed(v0, v1, p);

		// edge function delta
		int edge_a01 = v0.y - v1.y, edge_b01 = v1.x - v0.x;
		int edge_a12 = v1.y - v2.y, edge_b12 = v2.x - v1.x;
		int edge_a20 = v2.y - v0.y, edge_b20 = v0.x - v2.x;

		// top-left bias
		int bias_v01 = is_top_left(v0, v1) ? 0 : -1;
		int bias_v12 = is_top_left(v1, v2) ? 0 : -1;
		int bias_v20 = is_top_left(v2, v0) ? 0 : -1;

		// edge function increment
		int row_w0 = int(hp_w0 >> 8) + bias_v12;
		int row_w1 = int(hp_w1 >> 8) + bias_v20;
		int row_w2 = int(hp_w2 >> 8) + bias_v01;
		float fw0 = float(hp_w0 & 0xFF) / 255;
		float fw1 = float(hp_w1 & 0xFF) / 255;
		float fw2 = float(hp_w2 & 0xFF) / 255;

		int w0, w1, w2;
		float t_sum, t0, t1, t2;
		for (int y = block.min.y; y < block.max.y; y += 1)
		{
			w0 = row_w0;
			w1 = row_w1;
			w2 = row_w2;
			for (int x = block.min.x; x < block.max.x; x += 1)
			{
				if ((w0 | w1 | w2) >= 0) {
					t0 = w0 - bias_v12 + fw0;
					t1 = w1 - bias_v20 + fw1;
					t2 = w2 - bias_v01 + fw2;
					t_sum = t0 + t1 + t2;
					t0 /= t_sum;
					t1 /= t_sum;
					t2 /= t_sum;
					plot(x, y, pos0.z * t0 + pos1.z * t1 + pos2.z *t2, t0, t1, t2);
				}
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
