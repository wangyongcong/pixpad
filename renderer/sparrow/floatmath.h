#ifndef WYC_HEADER_FLOATMATH
#define WYC_HEADER_FLOATMATH

#include "xs_config.h"
#include "xs_float.h"

// fast C-Style convertion
#define to_int xs_ToInt
// fast round float to int
#define fast_round xs_RoundToInt
// fast floor float to int
#define fast_floor xs_FloorToInt
// fast ceil float to int
#define fast_ceil xs_CeilToInt
// fast covertion from real64 to .N fixed point
template<uint16 N>
inline int32 fast_to_fixed(real64 v)
{
	xs_CRoundToInt(val, _xs_doublemagic / (1 << N));
}

#endif // WYC_HEADER_FLOATMATH
