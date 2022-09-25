#pragma once
#include <stdint.h>

namespace wyc
{

inline unsigned align_up(unsigned x, unsigned align)
{
	assert((align & (align - 1)) == 0);
	return (x + align - 1) & ~(align - 1);
}

inline unsigned align_down(unsigned x, unsigned align)
{
	assert((align & (align - 1)) == 0);
	return x & ~(align - 1);
}

// swizzle sw*sh, 32bpp texel data from "src" to "dst" at position (dx, dy)
// "dst" is a 2D texture of dw*dh size, "dw" and "dh" should be padded to 64 boundaries
// "spitch" is the distance between rows in "src" image, in units of 32bpp texels
void swizzle_32bpp(uint32_t *dst, uint32_t dx, uint32_t dy, uint32_t dw, uint32_t dh, const uint32_t *src, uint32_t sw, uint32_t sh, uint32_t spitch);

// linearize sw*sh 32bpp texel data from "src" at position (dx, dy) to "dst"
// "src" is a 2D swizzlized texture, "sw" and "sh" should be padded to 64 boundaries
// "dst" is a 2D texture of dw*dh size, "dpitch" is the distance between rows in units of 32bpp texels
void linearize_32bpp(uint32_t *dst, uint32_t dw, uint32_t dh, uint32_t dpitch, const uint32_t *src, uint32_t sx, uint32_t sy, uint32_t sw, uint32_t sh);

// swizzle texel data from "src" to "dst". it use SIMD instruction to load/store data
// "sw" and "sh" should be 4 texel aligned
// "dst" should be aligned alloc at 64 bytes boundary, and contain enough space to hold sw*sh 32bpp texel data
// "dw" is the width of "dst" texture. It should be padded to 64 boundaries
void swizzle_32bpp_fast(uint32_t *dst, uint32_t dw, const uint32_t *src, uint32_t sw, uint32_t sh, uint32_t spitch);

// linearie texel data from "src" to "dst". it use SIMD instruction to load/store data
// "dw" and "dh" should be 4 texel aligned
// "src" should be aligned alloc at 64 bytes boundary, and contain enough space to hold sw*sh 32bpp texel data
// "dw" is the width of "dst" texture. It should be padded to 64 boundaries
void linearize_32bpp_fast(uint32_t *dst, uint32_t dw, uint32_t dh, uint32_t dpitch, const uint32_t *src, uint32_t sw);

} // namespace wyc
