#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_rasterizer.h"
#include <vector>

#define SPW_TILE_SIZE_MASK 3
#define SPW_TILE_SIZE_BITS 2
#define SPW_LOD_COUNT 3
#define SPW_LOD_MAX (SPW_LOD_COUNT - 1)
#define SPW_BLOCK_SIZE 64
#define SPW_BLOCK_SIZE_BITS 6

namespace wyc
{
	// Larrabee rasterizer
	// when individual coordinates are in [-2^p, 2^p-1], the result of edge function fits inside a 2*(p+2)-bit signed integer
	// e.g with 32-bit integer, the precision bits [p] is 32/2-2=14, coordinates should be in range [-16384, 16383]
	inline int coordinate_precision(int available_bits)
	{
		return available_bits / 2 - 2;
	}
	
	inline bool is_pod(int v)
	{
		return (v & (v-1)) == 0;
	}
	
	struct TriangleEdgeInfo
	{
		// 4x4 reject corner offsets
		mat4i rc_steps[3];
		// offset of reject corner to accept corner
		vec3i rc2ac;
		// edge function value at reject corner (high precision)
		int64_t rc_hp[3];
		vec2i dxdy[3];
		// top-left fill rule bias
		vec3i bias;
		vec3f tail;
	};

	// 1. vertices are in counter-clockwise order
	template<class Vector>
	void setup_triangle(TriangleEdgeInfo *edge, const Vector &vf0, const Vector &vf1, const Vector &vf2)
	{
		const vec2i vi[3] = {
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(vf1),
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(vf2),
			snap_to_subpixel<SPW_SUB_PIXEL_PRECISION>(vf0),
		};
		
		// block size with sub pixel precision
		constexpr int block_size_hp = SPW_BLOCK_SIZE << SPW_SUB_PIXEL_PRECISION;
		
		for(int i1=2, i2=0, j=0; i2 < 3; i1 = i2, i2 += 1, ++j)
		{
			auto &vi1 = vi[i1];
			auto &vi2 = vi[i2];
			vec2i rc;  // reject corner
			vec2i ac;  // offset of reject corner to accpet corner
			vec2i &dv = edge->dxdy[j];
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
					edge->rc_steps[j] = {
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
					edge->rc_steps[j] = {
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
					edge->rc_steps[j] = {
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
					edge->rc_steps[j] = {
						c3, c2, c1, 0,
						r1 + c3, r1 + c2, r1 + c1, r1,
						r2 + c3, r2 + c2, r2 + c1, r2,
						r3 + c3, r3 + c2, r3 + c1, r3,
					};
				}
			} // detect edge direction
			edge->rc2ac[j] = ac.x + ac.y;
			int64_t v = edge_function_fixed(vi1, vi2, rc);
			edge->rc_hp[j] = v;
			int b = is_top_left(vi1, vi2) ? 0 : -1;
			edge->bias[j] = b;
			edge->tail[j] = float((v & 0xFF) - b) / 255;
		}
	}
	
	template<class T>
	void scan_block(const TriangleEdgeInfo *edge, int row, int col, std::vector<PixelTile> &partial_blocks, T *shader)
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
	void scan_tile(const TriangleEdgeInfo *edge, int index, const vec3i &reject_value, std::vector<PixelTile> &partial_tiles, T *shader)
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
	void scan_pixel(const TriangleEdgeInfo *edge, int index, const vec3i &reject_value, T *shader)
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
			e[i] |= edge->rc_hp[i] & last_mask;
			e[i] += edge->rc2ac[i] << 7;
			e[i] += edge->bias[i];
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
		TriangleEdgeInfo edge;
		std::vector<PixelTile> partial_blocks;
		std::vector<PixelTile> partial_tiles;

		setup_triangle(&edge, v0, v1, v2);
		scan_block(&edge, block_row, block_col, partial_blocks, shader);
		for(auto &tile : partial_blocks) {
			scan_tile(&edge, tile.index, tile.reject, partial_tiles, shader);
		}
		for(auto &tile: partial_tiles) {
			scan_pixel(&edge, tile.index, tile.reject, shader);
		}
	}

} // namespace wyc
