#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_rasterizer.h"
#include <vector>

namespace wyc
{
	// Larrabee rasterizer
	// when individual coordinates are in range [-2^p, 2^p-1], the result of edge function fits inside 2*(p+2) bit signed integer
	// if we use a 32-bit integer to store edge function, then p=32/2-2=14, so the coordinates should be in range [-16384, 16383]
	// if we use a 24.8 sub-pixel precision, then p=24/2-2=10, so coordinates should be [-1024, 1024]
	inline int coordinate_precision(int available_bits)
	{
		return available_bits / 2 - 2;
	}
	
	inline bool is_pod(int v)
	{
		return (v & (v-1)) == 0;
	}
	
	// 1. vertices are in counter-clockwise order
	void setup_triangle(Triangle *prim, const vec2f *vf0, const vec2f *vf1, const vec2f *vf2)
	{
		const vec2i vi[3] = {
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(*vf1),
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(*vf2),
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(*vf0),
		};
		
		vec4i &bounding = prim->bounding;
		bounding.x = vi[0].x >> SPW_SUB_PIXEL_PRECISION;
		bounding.y = vi[0].y >> SPW_SUB_PIXEL_PRECISION;
		bounding.z = bounding.x;
		bounding.w = bounding.y;
		for(int i = 1; i < 3; ++i)
		{
			int x = vi[i].x >> SPW_SUB_PIXEL_PRECISION;
			int y = vi[i].y >> SPW_SUB_PIXEL_PRECISION;
			if(x < bounding.x)
				bounding.x = x;
			if(x > bounding.z)
				bounding.z = x;
			if(y < bounding.y)
				bounding.y = y;
			if(y > bounding.w)
				bounding.w = y;
		}
		
		// block size with sub pixel precision
		constexpr int block_size_hp = SPW_BLOCK_SIZE << SPW_SUB_PIXEL_PRECISION;
		
		for(int i1=2, i2=0, j=0; i2 < 3; i1 = i2, i2 += 1, ++j)
		{
			auto &vi1 = vi[i1];
			auto &vi2 = vi[i2];
			vec2i rc;  // reject corner
			vec2i ac;  // offset of reject corner to accpet corner
			vec2i &dv = prim->dxdy[j];
			dv.setValue(vi1.y - vi2.y, vi2.x - vi1.x);
			int dx = vi2.x - vi1.x;
			int dy = vi2.y - vi1.y;
			if(dx >= 0) {
				if(dy >= 0) {
					// point to 1st quadrant
					// the trivial reject point is at *
					// * -- o
					// |    |
					// o -- o
					rc.setValue(0, block_size_hp);
					ac.setValue(dv.x, -dv.y);
					int r1 = ac.y;
					int r2 = r1 * 2;
					int r3 = r1 * 3;
					int c1 = ac.x;
					int c2 = c1 * 2;
					int c3 = c1 * 3;
					prim->rc_steps[j] = {
						r3, r3 + c1, r3 + c2, r3 + c3,
						r2, r2 + c1, r2 + c2, r2 + c3,
						r1, r1 + c1, r1 + c2, r1 + c3,
						0, c1, c2, c3,
					};
				}
				else {
					// point to 4th quadrant
					// the trivial reject point is at *
					// o -- *
					// |    |
					// o -- o
					rc.setValue(block_size_hp, block_size_hp);
					ac.setValue(-dv.x, -dv.y);
					int r1 = ac.y;
					int r2 = r1 * 2;
					int r3 = r1 * 3;
					int c1 = ac.x;
					int c2 = c1 * 2;
					int c3 = c1 * 3;
					prim->rc_steps[j] = {
						r3 + c3, r3 + c2, r3 + c1, r3,
						r2 + c3, r2 + c2, r2 + c1, r2,
						r1 + c3, r1 + c2, r1 + c1, r1,
						c3, c2, c1, 0,
					};
				}
			}
			else {
				if(dy >= 0) {
					// point to 2nd quadrand
					// the trivial reject point is at *
					// o -- o
					// |    |
					// * -- o
					rc.setValue(0, 0);
					ac.setValue(dv.x, dv.y);
					int r1 = ac.y;
					int r2 = r1 * 2;
					int r3 = r1 * 3;
					int c1 = ac.x;
					int c2 = c1 * 2;
					int c3 = c1 * 3;
					prim->rc_steps[j] = {
						0, c1, c2, c3,
						r1, r1 + c1, r1 + c2, r1 + c3,
						r2, r2 + c1, r2 + c2, r2 + c3,
						r3, r3 + c1, r3 + c2, r3 + c3,
					};
				}
				else {
					// point to 3rd quadrand
					// the trivial reject point is at *
					// o -- o
					// |    |
					// o -- *
					rc.setValue(block_size_hp, 0);
					ac.setValue(-dv.x, dv.y);
					int r1 = ac.y;
					int r2 = r1 * 2;
					int r3 = r1 * 3;
					int c1 = ac.x;
					int c2 = c1 * 2;
					int c3 = c1 * 3;
					prim->rc_steps[j] = {
						c3, c2, c1, 0,
						r1 + c3, r1 + c2, r1 + c1, r1,
						r2 + c3, r2 + c2, r2 + c1, r2,
						r3 + c3, r3 + c2, r3 + c1, r3,
					};
				}
			} // detect edge direction
			prim->rc2ac[j] = ac.x + ac.y;
			int64_t v = edge_function_fixed(vi1, vi2, rc);
			prim->rc_hp[j] = v;
			int b = is_top_left(vi1, vi2) ? 0 : -1;
			prim->bias[j] = b;
			// .8 sub-pixel part with reversed bias
			prim->tail[j] = float(int((v + b) & 0xFF) - b) / 256;
//			prim->tail[j] = float((v & 0xFF) - b) / 255;
		}
	}
	
	void scan_block(RenderTarget *rt, const Triangle *prim, BlockArena *arena, TileQueue *full_blocks, TileQueue *partial_blocks)
	{
		// find blocks covered by primitive's aabb
		// block range: {beg_col, beg_row, end_col, end_row}
		vec4i bounding = prim->bounding;
		bounding.z += SPW_BLOCK_SIZE - 1;
		bounding.w += SPW_BLOCK_SIZE - 1;
		bounding /= SPW_BLOCK_SIZE;
		vec4i block_range = {-rt->x, -rt->y, -rt->x, -rt->y};
		block_range += bounding;
		block_range.x = std::max(block_range.x, 0);
		block_range.y = std::max(block_range.y, 0);
		block_range.z = std::min(block_range.z, rt->w);
		block_range.w = std::min(block_range.w, rt->h);
		
		// find partial covered blocks and full covered blocks
		constexpr int block_shift = SPW_SUB_PIXEL_PRECISION + SPW_BLOCK_SIZE_BITS;
		vec3i dy(prim->dxdy[0].y, prim->dxdy[1].y, prim->dxdy[2].y);
		vec3i dx(prim->dxdy[0].x, prim->dxdy[1].x, prim->dxdy[2].x);
		vec3i reject_row = {
			int(prim->rc_hp[0] >> block_shift),
			int(prim->rc_hp[1] >> block_shift),
			int(prim->rc_hp[2] >> block_shift),
		};
		reject_row += dx * block_range.x;
		reject_row += dy * block_range.y;
		vec3i reject, accept;
		// 4 ^ SPW_LOD_MAX = SPW_BLOCK_SIZE
		constexpr int shift = SPW_SUB_PIXEL_PRECISION + SPW_LOD_MAX * 2;
		int size = SPW_BLOCK_SIZE * SPW_BLOCK_SIZE * rt->pixel_size;
		assert(rt->pitch % SPW_BLOCK_SIZE == 0);
		unsigned row_size = rt->pitch * SPW_BLOCK_SIZE;
//		unsigned col_size = SPW_BLOCK_SIZE * SPW_BLOCK_SIZE * rt->pixel_size;
		char *row_start = rt->storage + block_range.y * row_size + block_range.x * size;
		for(int r = block_range.y; r < block_range.w; ++r, reject_row += dy, row_start += row_size)
		{
			reject = reject_row;
			char *storage = row_start;
			for(int c = block_range.x; c < block_range.z; ++c, reject += dx, storage += size)
			{
				if((reject.x | reject.y | reject.z) < 0)
					// trivial reject
					continue;
				accept = reject + prim->rc2ac;
				auto *block = arena->alloc();
				block->_next = nullptr;
				block->storage = storage;
				block->size = size;
				block->shift = shift;
				block->lod = 0;
				block->reject = reject;
				if((accept.x | accept.y | accept.z) > 0) {
					// trivial accept
					full_blocks->push(block);
				}
				else {
					// partial accept
					partial_blocks->push(block);
				}
			}
		} // loop of blocks
	}
	
	void scan_tile(const Triangle *prim, TileBlock *block, BlockArena *arena, TileQueue *full_tiles, TileQueue *partial_tiles)
	{
		int shift = block->shift;
		// todo: per triangle constant
		vec3i r = {
			int(prim->rc_hp[0] >> shift) & SPW_TILE_SIZE_MASK,
			int(prim->rc_hp[1] >> shift) & SPW_TILE_SIZE_MASK,
			int(prim->rc_hp[2] >> shift) & SPW_TILE_SIZE_MASK,
		};
		r.x += block->reject.x << SPW_TILE_SIZE_BITS;
		r.y += block->reject.y << SPW_TILE_SIZE_BITS;
		r.z += block->reject.z << SPW_TILE_SIZE_BITS;
		mat4i m0(r.x);
		mat4i m1(r.y);
		mat4i m2(r.z);
		m0 += prim->rc_steps[0];
		m1 += prim->rc_steps[1];
		m2 += prim->rc_steps[2];
		mat4i mask = m0 | m1;
		mask |= m2;
		int *x0 = (int*)m0.x;
		int *x1 = (int*)m1.x;
		int *x2 = (int*)m2.x;
		int *xm = (int*)mask.x;
		shift -= 2;
		int size = block->size >> 4;
		int lod = block->lod + 1;
		char* storage = block->storage;
		for(int i = 0; i < 16; ++i, storage += size)
		{
			if(xm[i] < 0)
				continue;
			vec3i reject(x0[i], x1[i], x2[i]);
			vec3i accept = reject + prim->rc2ac;
			TileBlock *b = arena->alloc();
			b->_next = nullptr;
			b->storage = storage;
			b->size = size;
			b->shift = shift;
			b->lod = lod;
			b->reject = reject;
			if((accept.x | accept.y | accept.z) > 0) {
				full_tiles->push(b);
			}
			else {
				partial_tiles->push(b);
			}
		}
	}
	
	template<class Shader>
	void fill_tile(const Triangle *prim, TileBlock *tile, Shader shader)
	{
		
	}
	
	void draw_tile(const Triangle *prim, TileBlock *tile, PixelShader shader)
	{
		constexpr int last_shift = SPW_SUB_PIXEL_PRECISION + SPW_TILE_SIZE_BITS;
		constexpr int last_mask = (1 << last_shift) - 1;
		int64_t e[3];
		for(int i=0; i<3; ++i)
		{
			e[i] = tile->reject[i];
			e[i] <<= last_shift;
			// recover the full precision edge function at reject corner
			e[i] |= prim->rc_hp[i] & last_mask;
			// edge function at pixel center (0.5 sub-pixel precision)
			e[i] += prim->rc2ac[i] << 7;
			// add the top-left rule bias
			e[i] += prim->bias[i];
		}
		mat4i m0 = int(e[0] >> SPW_SUB_PIXEL_PRECISION);
		mat4i m1 = int(e[1] >> SPW_SUB_PIXEL_PRECISION);
		mat4i m2 = int(e[2] >> SPW_SUB_PIXEL_PRECISION);
		m0 += prim->rc_steps[0];
		m1 += prim->rc_steps[1];
		m2 += prim->rc_steps[2];
		mat4i mask = m0 | m1;
		mask |= m2;
		int *x0 = (int*)m0.x;
		int *x1 = (int*)m1.x;
		int *x2 = (int*)m2.x;
		int *xm = (int*)mask.x;
		constexpr int PIXEL_SIZE = 4;
		char *dst = tile->storage;
		for(int i = 0; i < 16; ++i, ++xm) {
			if(*xm < 0)
				continue;
			vec3f w(x0[i], x1[i], x2[i]);
			w += prim->tail;
			float sum = w.x + w.y + w.z;
			w /= sum;
			shader(dst + (i*PIXEL_SIZE), w);
		}
	}
	
	template<class T>
	void scan_block(const Triangle *edge, int row, int col, std::vector<PixelTile> &partial_blocks, T *shader)
	{
		constexpr int block_shift = SPW_SUB_PIXEL_PRECISION + SPW_BLOCK_SIZE_BITS;
		vec3i dy(edge->dxdy[0].y, edge->dxdy[1].y, edge->dxdy[2].y);
		vec3i dx(edge->dxdy[0].x, edge->dxdy[1].x, edge->dxdy[2].x);
		vec3i reject_row = {
			int(edge->rc_hp[0] >> block_shift),
			int(edge->rc_hp[1] >> block_shift),
			int(edge->rc_hp[2] >> block_shift),
		};
		vec3i reject, accept;
		for(int r = 0, i = 0; r < row; ++r, reject_row += dy)
		{
			reject = reject_row;
			for(int c = 0; c < col; ++c, ++i, reject += dx)
			{
				if((reject.x | reject.y | reject.z) < 0)
					// trivial reject
					continue;
				accept = reject + edge->rc2ac;
				if((accept.x | accept.y | accept.z) > 0) {
					// trivial accept
					shader->fill_block(i, reject);
				}
				else {
					// partial accept
					int index = (i << ((SPW_LOD_MAX * 4) + SPW_LOD_BITS)) + SPW_LOD_MAX;
					partial_blocks.emplace_back(index, reject);
					shader->fill_partial_block(i, reject);
				}
			}
		} // loop of blocks
	}
	
	template<class T>
	void scan_tile(const Triangle *edge, int index, const vec3i &reject_value, std::vector<PixelTile> &partial_tiles, T *shader)
	{
		int lod = index & SPW_LOD_MASK;
		assert(lod > 0);
		int tile_shift = SPW_SUB_PIXEL_PRECISION + lod * 2;
		// todo: per triangle constant
		vec3i r = {
			int(edge->rc_hp[0] >> tile_shift) & SPW_TILE_SIZE_MASK,
			int(edge->rc_hp[1] >> tile_shift) & SPW_TILE_SIZE_MASK,
			int(edge->rc_hp[2] >> tile_shift) & SPW_TILE_SIZE_MASK,
		};
		r.x += reject_value.x << SPW_TILE_SIZE_BITS;
		r.y += reject_value.y << SPW_TILE_SIZE_BITS;
		r.z += reject_value.z << SPW_TILE_SIZE_BITS;
		mat4i m0(r.x);
		mat4i m1(r.y);
		mat4i m2(r.z);
		m0 += edge->rc_steps[0];
		m1 += edge->rc_steps[1];
		m2 += edge->rc_steps[2];
		mat4i mask = m0 | m1;
		mask |= m2;
		int *x0 = (int*)m0.x;
		int *x1 = (int*)m1.x;
		int *x2 = (int*)m2.x;
		int *xm = (int*)mask.x;
		int istep = 1 << ((lod - 1) * 4 + SPW_LOD_BITS);
		// decrease LOD level
		index -= 1;
		for(int i = 0; i < 16; ++i, index += istep)
		{
			if(xm[i] < 0)
				continue;
			vec3i reject(x0[i], x1[i], x2[i]);
			vec3i accept = reject + edge->rc2ac;
			if((accept.x | accept.y | accept.z) > 0)
				shader->fill_tile(index, reject);
			else if(lod > 1) {
				scan_tile(edge, index, reject, partial_tiles, shader);
				shader->fill_partial_tile(index, reject);
			}
			else {
				partial_tiles.emplace_back(index, reject);
				shader->fill_partial_tile(index, reject);
			}
		}
	}
	
	template<class T>
	void scan_pixel(const Triangle *edge, int index, const vec3i &reject_value, T *shader)
	{
		int64_t e[3];
		vec3f tail;
		constexpr int last_shift = SPW_SUB_PIXEL_PRECISION + SPW_TILE_SIZE_BITS;
		constexpr int last_mask = (1 << last_shift) - 1;
		for(int i=0; i<3; ++i)
		{
			e[i] = reject_value[i];
			e[i] <<= last_shift;
			// todo: per triangle constant
			// recover the full precision edge function at reject corner
			e[i] |= edge->rc_hp[i] & last_mask;
			// edge function at pixel center (0.5 sub-pixel precision)
			e[i] += edge->rc2ac[i] << 7;
			// add the top-left rule bias
			e[i] += edge->bias[i];
			// .8 sub-pixel part
			tail[i] = float((e[i] - edge->bias[i]) & 0xFF) / 255;
		}
		mat4i m0 = int(e[0] >> SPW_SUB_PIXEL_PRECISION);
		mat4i m1 = int(e[1] >> SPW_SUB_PIXEL_PRECISION);
		mat4i m2 = int(e[2] >> SPW_SUB_PIXEL_PRECISION);
		m0 += edge->rc_steps[0];
		m1 += edge->rc_steps[1];
		m2 += edge->rc_steps[2];
		mat4i mask = m0 | m1;
		mask |= m2;
		int *x0 = (int*)m0.x;
		int *x1 = (int*)m1.x;
		int *x2 = (int*)m2.x;
		int *xm = (int*)mask.x;
		for(int i = 0; i < 16; ++i, ++xm) {
			if(*xm < 0)
				continue;
			vec3f w(x0[i], x1[i], x2[i]);
			w += tail;
			float sum = w.x + w.y + w.z;
			w /= sum;
			shader->shade(index, i, w);
		}
	}
	
	void fill_triangle_larrabee(int block_row, int block_col, const vec2f &v0, const vec2f &v1, const vec2f &v2, ITileShader *shader)
	{
		Triangle edge;
		std::vector<PixelTile> partial_blocks;
		std::vector<PixelTile> partial_tiles;

		setup_triangle(&edge, &v0, &v1, &v2);
		scan_block(&edge, block_row, block_col, partial_blocks, shader);
		for(auto &tile : partial_blocks) {
			scan_tile(&edge, tile.index, tile.reject, partial_tiles, shader);
		}
		for(auto &tile: partial_tiles) {
			scan_pixel(&edge, tile.index, tile.reject, shader);
		}
	}

} // namespace wyc
