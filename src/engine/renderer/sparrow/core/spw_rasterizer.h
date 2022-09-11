#pragma once
#include <vector>
#include <ImathBox.h>
#include "floatmath.h"

#define SPW_SUB_PIXEL_PRECISION 8
#define SPW_TILE_SIZE_MASK 3
#define SPW_TILE_SIZE_BITS 2
#define SPW_LOD_COUNT 3
#define SPW_LOD_MAX (SPW_LOD_COUNT - 1)
#define SPW_BLOCK_SIZE 64
#define SPW_BLOCK_SIZE_BITS 6

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
#ifdef DEBUG
		if (area == 0)
			return vec3f(0, 0, 0);
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

	// v1v0 cross product v2v0
	inline int64_t edge_function_fixed(const vec2i &v0, const vec2i &v1, const vec2i &v2)
	{
		return int64_t(v1.x - v0.x) * int64_t(v2.y - v0.y) - int64_t(v1.y - v0.y) * int64_t(v2.x - v0.x);
	}
	
	inline int64_t edge_function_fixed(const vec2i &v0, const vec2i &v1)
	{
		return int64_t(v0.x) * int64_t(v1.y) - int64_t(v0.y) * int64_t(v1.x);
	}

	template<typename Vertex>
	inline Vertex interpolate(const Vertex &v0, const Vertex &v1, const Vertex &v2, float t0, float t1, float t2)
	{
		return v0 * t0 + v1 * t1 + v2 * t2;
	}
	
	// Triangle winding: 1 clockwise; -1 counter clockwise
	template<class T, int Winding=-1>
	bool is_backface(const T &p0, const T &p1, const T &p2) {
		// p = {x, y, z, w}
		vec2f v10(p0[0] - p1[0], p0[1] - p1[1]);
		vec2f v12(p2[0] - p1[0], p2[1] - p1[1]);
		float d = v10.cross(v12) * Winding + 0.001f;
		return d <= 0;

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

		// top-left bias
		int bias_v01 = is_top_left(v0, v1) ? 0 : -1;
		int bias_v12 = is_top_left(v1, v2) ? 0 : -1;
		int bias_v20 = is_top_left(v2, v0) ? 0 : -1;
		
		int64_t hp_w0 = edge_function_fixed(v1, v2, p) + bias_v12;
		int64_t hp_w1 = edge_function_fixed(v2, v0, p) + bias_v20;
		int64_t hp_w2 = edge_function_fixed(v0, v1, p) + bias_v01;

		// edge function delta
		int edge_a01 = v0.y - v1.y, edge_b01 = v1.x - v0.x;
		int edge_a12 = v1.y - v2.y, edge_b12 = v2.x - v1.x;
		int edge_a20 = v2.y - v0.y, edge_b20 = v0.x - v2.x;

		// edge function increment
		int row_w0 = int(hp_w0 >> 8);
		int row_w1 = int(hp_w1 >> 8);
		int row_w2 = int(hp_w2 >> 8);

		// .8 sub pixel part which is constant during iteration
		float fw0 = ((hp_w0 & 0xFF) - bias_v12) / 255.0f;
		float fw1 = ((hp_w1 & 0xFF) - bias_v20) / 255.0f;
		float fw2 = ((hp_w2 & 0xFF) - bias_v01) / 255.0f;

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
					t0 = w0 + fw0;
					t1 = w1 + fw1;
					t2 = w2 + fw2;
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
	
/*
 * Larrabee rasterization method
 * render traget is padded to 64*64 blocks
 * each block is 4*4 tile in Morton order
 */

#define SPW_LOD_BITS 2
#define SPW_LOD_MASK 3

	struct Triangle
	{
		// bouding box of triangle
		vec4i bounding;
		// 4x4 reject corner offsets
		alignas(64) mat4i rc_steps[3];
		// offset of reject corner to accept corner
		vec3i rc2ac;
		// edge function value at reject corner (high precision)
		int64_t rc_hp[3];
		// edge function delta in x/y direction
		vec2i dxdy[3];
		// top-left fill rule bias
		vec3i bias;
		// low bits of full precision edge function value
		vec3f tail;
		// pixcel center offset
		vec3i center_offset;
	};
	
	template<class T>
	class CMemoryArena
	{
		struct Node {
			T *_next;
		};
		
	public:
		CMemoryArena(unsigned capacity=0)
		: m_objs(nullptr)
		, m_free(nullptr)
		, m_bucket(nullptr)
		, m_bucket_count(0)
		, m_bucket_size(0)
		{
			if(capacity > 0)
				reserve(capacity);
		}
		
		~CMemoryArena() {
			_free_bucket();
			m_bucket = nullptr;
			m_objs = nullptr;
			m_free = nullptr;
		}
		
		void reserve(unsigned capacity)
		{
			unsigned bucket_size = sizeof(T) * capacity;
			if(m_bucket) {
				if(bucket_size <= m_bucket_size)
					return;
				_free_bucket();
			}
			m_bucket_size = bucket_size;
			m_bucket_count = 1;
			_reset_bucket();
		}
		
		void clear() {
			if(m_bucket_count <= 1)
			{
				m_objs = (T*)(m_bucket - m_bucket_size);
				m_free = nullptr;
				return;
			}
			_free_bucket();
			m_bucket_size *= m_bucket_count;
			m_bucket_count = 1;
			_reset_bucket();
		}
		
		T* alloc() {
			if(m_free)
			{
				T *obj = m_free;
				m_free = ((Node*)obj)->_next;
				return obj;
			}
			if((char*)m_objs >= m_bucket)
			{
				char *buf = new char[m_bucket_size + sizeof(void*)];
				char *node = buf + m_bucket_size;
				*(char**)node = m_bucket;
				m_bucket = node;
				m_objs = (T*)buf;
				m_bucket_count += 1;
			}
			return m_objs++;
		}
		
		void free(T *obj)
		{
			((Node*)obj)->_next = m_free;
			m_free = obj;
		}
		
		unsigned bucket_count() const {
			return m_bucket_count;
		}
		
	private:
		void _reset_bucket() {
			char *buf = new char[m_bucket_size + sizeof(void*)];
			m_bucket = buf + m_bucket_size;
			*(char**)m_bucket = nullptr;
			m_objs = (T*)buf;
			m_free = nullptr;
		}
		
		void _free_bucket() {
			while(m_bucket) {
				char *buf = m_bucket - m_bucket_size;
				m_bucket = *(char**)m_bucket;
				delete [] buf;
			}
		}
		
		T *m_objs;
		T *m_free;
		char *m_bucket;
		unsigned m_bucket_count;
		unsigned m_bucket_size;
	};
	
	struct TileBlock
	{
		TileBlock *_next;
		char *storage;
		int size;
		int shift;
		int lod;
		vec3i reject;
	};
	
	typedef CMemoryArena<TileBlock> BlockArena;
	
	struct RenderTarget
	{
		char *storage;
		unsigned pixel_size;
		unsigned pitch;
		int w, h;
		int x, y;
	};
	
	struct TileQueue
	{
		TileBlock *head;
		TileBlock **tail;
		
		TileQueue() {
			head = nullptr;
			tail = &head;
		}
		
		inline void push(TileBlock *b) {
			*tail = b;
			tail = &b->_next;
		}
		
		inline TileBlock* pop() {
			if(!head)
				return nullptr;
			auto *b = head;
			head = head->_next;
			if(!head)
				tail = &head;
			return b;
		}
		
		inline void join(TileQueue *queue)
		{
			if(queue->head) {
				*tail = queue->head;
				tail = queue->tail;
				queue->clear();
			}
		}
		
		inline void clear() {
			head = nullptr;
			tail = &head;
		}
		
		inline operator bool () const {
			return head != nullptr;
		}
	};

	void setup_triangle(Triangle *edge, const vec2f *vf0, const vec2f *vf1, const vec2f *vf2);
	// search all blocks covered by triangle
	unsigned scan_block(RenderTarget *rt, const Triangle *tri, BlockArena *arena, TileQueue *full_blocks, TileQueue *partial_blocks);
	// search all full/partial tiles covered by triangle
	void scan_tile(const Triangle *prim, TileBlock *block, BlockArena *arena, TileQueue *full_tiles, TileQueue *partial_tiles);
	// divide full-covered blocks into tiles
	void split_tile(const Triangle *prim, BlockArena *arena, TileBlock *block, TileQueue *full_tiles);

	typedef void (*PixelShader) (char *dst, const vec3f &w);
	// draw partial-covered tile
	void draw_tile(const Triangle *prim, TileBlock *tile, PixelShader shader);
	// draw full-covered tile
	void fill_tile(const Triangle *prim, TileBlock *tile, PixelShader shader);

} // namespace wyc
