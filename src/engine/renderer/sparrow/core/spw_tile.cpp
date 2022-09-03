#include <cassert>
#include <emmintrin.h>
#include "spw_tile.h"

namespace wyc
{
// Pixel order: 4*4 in 16*16 in 64*64
//
//        (bit index)    c  b a 9 8 7 6 5 4 3 2 1 0
// index        = tile_idx:y5y4x5x4y3y2x3x2y1y0x1x0
//              = index_x_tile | index_y
//
// with
//
//        (bit index)    c  b a 9 8 7 6 5 4 3 2 1 0
// index_x_tile = tile_idx:____x5x4____x3x2____x1x0
// index_y      = _________y5y4____y3y2____y1y0____

#define SWIZZLE_INNER_TILE_W 4
#define SWIZZLE_INNER_TILE_H 4
#define SWIZZLE_OUTER_MOST_TILE_W 64
#define SWIZZLE_OUTER_MOST_TILE_H 64

static uintptr_t swizzle_x_tile(uintptr_t x)
{
    return ((x & 0x03) << 0) | ((x & 0x0c) << 2) | ((x & 0x30) << 4) | ((x & ~0x3f) << 6);
}

static uintptr_t swizzle_y(uintptr_t y)
{
    return ((y & 0x03) << 2) | ((y & 0x0c) << 4) | ((y & 0x30) << 6);
}

void swizzle_32bpp(uint32_t *dst, uint32_t dx, uint32_t dy, uint32_t dw, uint32_t dh, const uint32_t *src, uint32_t sw, uint32_t sh, uint32_t spitch)
{
	uintptr_t mask_x = swizzle_x_tile(~0u);
	uintptr_t mask_y = swizzle_y(~0u);
	uintptr_t incr_x0 = swizzle_x_tile(align_up(dw, SWIZZLE_OUTER_MOST_TILE_W));
	uintptr_t offs_x0 = swizzle_x_tile(dx) + incr_x0 * (dy / SWIZZLE_OUTER_MOST_TILE_H);
	uintptr_t offs_y = swizzle_y(dy);
	
	for(uint32_t y = 0; y < sh; ++y)
	{
		auto dst_line = dst + offs_y;
		auto offs_x = offs_x0;
		for(uint32_t x = 0; x < sw; ++x)
		{
			dst_line[offs_x] = src[x];
			offs_x = (offs_x - mask_x) & mask_x;
		}
		src += spitch;
		offs_y = (offs_y - mask_y) & mask_y;
		if(!offs_y)
			offs_x0 += incr_x0;
	}
}

void linearize_32bpp(uint32_t *dst, uint32_t dw, uint32_t dh, uint32_t dpitch, const uint32_t *src, uint32_t sx, uint32_t sy, uint32_t sw, uint32_t sh)
{
	uintptr_t mask_x = swizzle_x_tile(~0u);
	uintptr_t mask_y = swizzle_y(~0u);
	uintptr_t incr_x0 = swizzle_x_tile(align_up(sw, SWIZZLE_OUTER_MOST_TILE_W));
	uintptr_t offs_x0 = swizzle_x_tile(sx) + incr_x0 * (sy / SWIZZLE_OUTER_MOST_TILE_H);
	uintptr_t offs_y = swizzle_y(sy);
	
	for(uint32_t y = 0; y < dh; ++y)
	{
		auto src_line = src + offs_y;
		auto offs_x = offs_x0;
		for(uint32_t x = 0u; x < dw; ++x)
		{
			dst[x] = src_line[offs_x];
			offs_x = (offs_x - mask_x) & mask_x;
		}
		dst += dpitch;
		offs_y = (offs_y - mask_y) & mask_y;
		if(!offs_y)
			offs_x0 += incr_x0;
	}
}

typedef __m128i pixel4_t;

static inline pixel4_t loadpixel4u(const uint32_t *src)
{
    // unaligned load
    return _mm_loadu_si128((const __m128i *) src);
}

static void storepixel4u(uint32_t *dest, pixel4_t vals)
{
    // unaligned store
    _mm_storeu_si128((__m128i *) dest, vals);
}

static inline pixel4_t loadpixel4(const uint32_t *src)
{
    // aligned load
    return _mm_load_si128((const __m128i *) src);
}

static void storepixel4(uint32_t *dest, pixel4_t vals)
{
    // aligned store
    _mm_store_si128((__m128i *) dest, vals);
}

void swizzle_32bpp_fast(uint32_t *dst, uint32_t dw, const uint32_t *src, uint32_t sw, uint32_t sh, uint32_t spitch)
{
	assert(sw % SWIZZLE_INNER_TILE_W == 0);
	assert(sh % SWIZZLE_INNER_TILE_H == 0);

	auto mask_x = swizzle_x_tile(~0u * SWIZZLE_INNER_TILE_W);
	auto mask_y = swizzle_y(~0u * SWIZZLE_INNER_TILE_H);
	auto offs_x0 = swizzle_x_tile(0);
	auto offs_y = offs_x0;
	auto incr_y = swizzle_x_tile(align_up(dw, SWIZZLE_OUTER_MOST_TILE_W));
		
	for(uint32_t y = 0; y < sh; y += SWIZZLE_INNER_TILE_H) {
		auto dst_line = dst + offs_y;
		auto offs_x = offs_x0;
		for(uint32_t x = 0; x < sw; x += SWIZZLE_INNER_TILE_W)
		{
			auto src_line = src + x;
			pixel4_t row0 = loadpixel4u(src_line);
			pixel4_t row1 = loadpixel4u(src_line + spitch);
			pixel4_t row2 = loadpixel4u(src_line + spitch * 2);
			pixel4_t row3 = loadpixel4u(src_line + spitch * 3);
			
			auto dst_tile = dst_line + offs_x;
			storepixel4(dst_tile +  0, row0);
			storepixel4(dst_tile +  4, row1);
			storepixel4(dst_tile +  8, row2);
			storepixel4(dst_tile + 12, row3);
			
			offs_x = (offs_x - mask_x) & mask_x;
		}
		src += spitch * SWIZZLE_INNER_TILE_H;
		offs_y = (offs_y - mask_y) & mask_y;
		if(!offs_y)
			offs_x0 += incr_y;
	}
}

void linearize_32bpp_fast(uint32_t *dst, uint32_t dw, uint32_t dh, uint32_t dpitch, const uint32_t *src, uint32_t sw)
{
	assert(dw % SWIZZLE_INNER_TILE_W == 0);
	assert(dh % SWIZZLE_INNER_TILE_H == 0);

	auto mask_x = swizzle_x_tile(~0u * SWIZZLE_INNER_TILE_W);
	auto mask_y = swizzle_y(~0u * SWIZZLE_INNER_TILE_H);
	auto offs_x0 = swizzle_x_tile(0);
	auto offs_y = offs_x0;
	auto incr_y = swizzle_x_tile(align_up(sw, SWIZZLE_OUTER_MOST_TILE_W));
	
	for(uint32_t y = 0; y < dh; y += SWIZZLE_INNER_TILE_H) {
		auto src_line = src + offs_y;
		auto offs_x = offs_x0;
		for(uint32_t x = 0; x < dw; x += SWIZZLE_INNER_TILE_W)
		{
			auto src_tile = src_line + offs_x;
			pixel4_t row0 = loadpixel4(src_tile +  0);
			pixel4_t row1 = loadpixel4(src_tile +  4);
			pixel4_t row2 = loadpixel4(src_tile +  8);
			pixel4_t row3 = loadpixel4(src_tile + 12);

			auto dst_line = dst + x;
			storepixel4u(dst_line, row0);
			storepixel4u(dst_line + dpitch, row1);
			storepixel4u(dst_line + dpitch * 2, row2);
			storepixel4u(dst_line + dpitch * 3, row3);

			offs_x = (offs_x - mask_x) & mask_x;
		}
		dst += dpitch * SWIZZLE_INNER_TILE_H;
		offs_y = (offs_y - mask_y) & mask_y;
		if(!offs_y)
			offs_x0 += incr_y;
	}
}

void swizzle_32bpp_bruteforce(uint32_t *dst, uint32_t dw, uint32_t dh, const uint32_t *src, uint32_t sw, uint32_t sh, uint32_t spitch)
{
	constexpr unsigned s0 = 64 * 64;
	constexpr unsigned s1 = 16 * 16;
	constexpr unsigned s2 = 4 * 4;
	
	assert(dw % 64 == 0);
	auto dw0 = dw / 64;

	for(uint32_t y=0; y<sh; ++y)
	{
		auto y0 = y / 64;
		auto y1 = (y % 64) / 16;
		auto y2 = ((y % 64) % 16) / 4;
		auto y3 = ((y % 64) % 16) % 4;

		for(uint32_t x=0; x<sw; ++x)
		{
			auto x0 = x / 64;
			auto x1 = (x % 64) / 16;
			auto x2 = ((x % 64) % 16) / 4;
			auto x3 = ((x % 64) % 16) % 4;

			auto i = (y0 * dw0 + x0) * s0 + (y1 * 4 + x1) * s1 + (y2 * 4 + x2) * s2 + (y3 * 4 + x3);
			dst[i] = src[x];
		}
		src += spitch;
	}
}

void linearize_32bpp_bruteforce(uint32_t *dst, uint32_t dw, uint32_t dh, uint32_t dpitch, const uint32_t *src, uint32_t sw, uint32_t sh)
{
	constexpr unsigned s0 = 64 * 64;
	constexpr unsigned s1 = 16 * 16;
	constexpr unsigned s2 = 4 * 4;
	
	assert(sw % 64 == 0);
	auto sw0 = sw / 64;
	
	for(uint32_t y=0; y<dh; ++y)
	{
		auto y0 = y / 64;
		auto y1 = (y % 64) / 16;
		auto y2 = ((y % 64) % 16) / 4;
		auto y3 = ((y % 64) % 16) % 4;

		for(uint32_t x=0; x<dw; ++x)
		{
			auto x0 = x / 64;
			auto x1 = (x % 64) / 16;
			auto x2 = ((x % 64) % 16) / 4;
			auto x3 = ((x % 64) % 16) % 4;

			auto i = (y0 * sw0 + x0) * s0 + (y1 * 4 + x1) * s1 + (y2 * 4 + x2) * s2 + (y3 * 4 + x3);
			dst[x] = src[i];
		}
		dst += dpitch;
	}
}

} // namespace wyc
