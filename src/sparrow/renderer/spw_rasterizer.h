#pragma once
#include <ImathExc.h>
#include <ImathBox.h>
#include "floatmath.h"

namespace wyc
{
	// bresenham line rasterization
	template<typename Plotter>
	void line_bresenham(Plotter &plot, int x0, int y0, int x1, int y1)
	{
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

	template<typename Plotter, typename Vec>
	void draw_line(Plotter &plot, const Vec &v1, const Vec &v2)
	{
		line_bresenham(plot, fast_floor(v1.x), fast_floor(v1.y), fast_floor(v2.x), fast_floor(v2.y));
	}

	// calculate the signed-area * 2 of triangle (v0, v1, v2)
	// return positive value if counter-clockwise, 
	// negative value for clockwise, 
	// 0 for degenerated triangle (v0, v1, v2 are collinear)
	inline float triangle_edge_function(const vec2f &v0, const vec2f &v1, const vec2f &v2)
	{
		return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
	}

	inline bool is_inside_triangle(const vec2f &p, const vec2f &v0, const vec2f &v1, const vec2f &v2)
	{
		float edge01 = triangle_edge_function(v0, v1, p);
		float edge12 = triangle_edge_function(v1, v2, p);
		float edge20 = triangle_edge_function(v2, v0, p);
		return edge01 >= 0 && edge12 >= 0 && edge20 >= 0;
	}

	inline vec3f barycentric_coord(const vec2f &p, const vec2f &v0, const vec2f &v1, const vec2f &v2)
	{
		float x = triangle_edge_function(v1, v2, p);
		float y = triangle_edge_function(v2, v0, p);
		float z = triangle_edge_function(v0, v1, p);
		float area = x + y + z;
#ifdef _DEBUG
		if (area == float(0))
			throw Imath::NullVecExc("Null barycentric vector.");
#endif			
		return vec3f(x / area, y / area, z / area);
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
	inline vec2i snap_to_subpixel(const Position &v)
	{
		return vec2i(fast_to_fixed<N>(v.x), fast_to_fixed<N>(v.y));
	}

	inline int64_t edge_function_fixed(const vec2i &v0, const vec2i &v1, const vec2i &v2)
	{
		return int64_t(v1.x - v0.x) * int64_t(v2.y - v0.y) - int64_t(v1.y - v0.y) * int64_t(v2.x - v0.x);
	}

	template<typename Vertex>
	inline Vertex interpolate(const Vertex &v0, const Vertex &v1, const Vertex &v2, float t0, float t1, float t2)
	{
		return v0 * t0 + v1 * t1 + v2 * t2;
	}


#define ASSERT_INSIDE(v, r) assert(v.x >= -r && v.x < r && v.y >= -r && v.y < r)

	// fill triangle {pos0, pos1, pos2} in counter-clockwise
	// position vector: should be 3D vector {x, y, z}
	// plot: functor with following declaration 
	//   void plot(int x, int y, float z, t0, t1, t2)
	template<typename Vector, typename Plotter>
	void fill_triangle(const box2i &block, const Vector &pos0, const Vector &pos1, const Vector &pos2, Plotter &plot)
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
		vec2i v0 = snap_to_subpixel<8>(pos0);
		vec2i v1 = snap_to_subpixel<8>(pos1);
		vec2i v2 = snap_to_subpixel<8>(pos2);

		// initial edge function with high precision
		// sample point is at pixel center
		vec2i p = block.min;
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

		// .8 sub pixel part which is constant during iteration
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
					t0 = (w0 - bias_v12) + fw0;
					t1 = (w1 - bias_v20) + fw1;
					t2 = (w2 - bias_v01) + fw2;
					t_sum = t0 + t1 + t2;
					t0 /= t_sum;
					t1 /= t_sum;
					t2 /= t_sum;
					plot(x, y, pos0.z * t0 + pos1.z * t1 + pos2.z * t2, t0, t1, t2);
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

#define _T_QUAD(_t, _v, _bias, _sub) {\
	(_t).x = ((_v).x - (_bias)) + (_sub); \
	(_t).y = ((_v).y - (_bias)) + (_sub); \
	(_t).z = ((_v).z - (_bias)) + (_sub); \
	(_t).w = ((_v).w - (_bias)) + (_sub); \
}

#define _INC_QUAD(_v, _step) {\
	(_v).x += (_step); (_v).y += (_step); (_v).z += (_step); (_v).w += (_step); \
}

	template<typename Vector, typename Plotter>
	void fill_triangle_quad(const box2i &block, const Vector &pos0, const Vector &pos1, const Vector &pos2, Plotter &plot)
	{
		// 11.8 sub pixel precision
		// max render target is 2048 x 2048
		// coordinate must be inside [-1024, 1024)
		ASSERT_INSIDE(block.min, 1024);
		ASSERT_INSIDE(block.max, 1024);
		ASSERT_INSIDE(pos0, 1024);
		ASSERT_INSIDE(pos1, 1024);
		ASSERT_INSIDE(pos2, 1024);
		// block size must be 2x2 at least
		assert((block.max.x - block.min.x) > 1 && (block.max.y - block.min.y) > 1);
		// block size must be even
		assert((block.max.x - block.min.x) % 2 == 0 && (block.max.y - block.min.y) % 2 == 0);

		// snap to .8 sub pixel
		vec2i v0 = snap_to_subpixel<8>(pos0);
		vec2i v1 = snap_to_subpixel<8>(pos1);
		vec2i v2 = snap_to_subpixel<8>(pos2);

		// initial edge function with high precision
		// sample point is at pixel center
		vec2i p = block.min;
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
		vec4i row_w0, row_w1, row_w2;

		row_w0.x = int(hp_w0 >> 8) + bias_v12;
		row_w0.y = row_w0.x + edge_a12;
		row_w0.z = row_w0.x + edge_b12;
		row_w0.w = row_w0.z + edge_a12;

		row_w1.x = int(hp_w1 >> 8) + bias_v20;
		row_w1.y = row_w1.x + edge_a20;
		row_w1.z = row_w1.x + edge_b20;
		row_w1.w = row_w1.z + edge_a20;

		row_w2.x = int(hp_w2 >> 8) + bias_v01;
		row_w2.y = row_w2.x + edge_a01;
		row_w2.z = row_w2.x + edge_b01;
		row_w2.w = row_w2.z + edge_a01;

		edge_a12 *= 2;
		edge_a20 *= 2;
		edge_a01 *= 2;
		edge_b12 *= 2;
		edge_b20 *= 2;
		edge_b01 *= 2;

		// .8 sub pixel part which is constant during iteration
		float fw0 = float(hp_w0 & 0xFF) / 255;
		float fw1 = float(hp_w1 & 0xFF) / 255;
		float fw2 = float(hp_w2 & 0xFF) / 255;

		//int w0, w1, w2;
		vec4i w0, w1, w2, flag;
		//float t_sum, t0, t1, t2;
		vec4f t_sum, t0, t1, t2;
		
		for (int y = block.min.y; y < block.max.y; y += 2)
		{
			w0 = row_w0;
			w1 = row_w1;
			w2 = row_w2;
			for (int x = block.min.x; x < block.max.x; x += 2)
			{
				flag = w0 | w1 | w2;
				if ((flag.x & flag.y & flag.z & flag.w) >= 0) {
					_T_QUAD(t0, w0, bias_v12, fw0);
					_T_QUAD(t1, w1, bias_v20, fw1);
					_T_QUAD(t2, w2, bias_v01, fw2);
					t_sum = t0 + t1 + t2;
					t0 /= t_sum;
					t1 /= t_sum;
					t2 /= t_sum;
					plot(x, y, (t0 * pos0.z) + (t1 * pos1.z) + (t2 * pos2.z), flag, t0, t1, t2);
				}
				else {
					flag = { 0, 0, 0, 0 };
				}
				_INC_QUAD(w0, edge_a12);
				_INC_QUAD(w1, edge_a20);
				_INC_QUAD(w2, edge_a01);
			}
			_INC_QUAD(row_w0, edge_b12);
			_INC_QUAD(row_w1, edge_b20);
			_INC_QUAD(row_w2, edge_b01);
		}
	}

} // namespace wyc
