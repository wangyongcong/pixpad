#ifndef WYC_HEADER_UTIL
#define WYC_HEADER_UTIL

#include <cstdint>

namespace wyc
{

typedef float float32_t;

inline bool have_state(unsigned st, unsigned flag) {
	return flag == (st & flag);
}
inline void add_state(unsigned &st, unsigned flag) {
	st |= flag;
}
inline void set_state(unsigned &st, unsigned mask, unsigned flag) {
	st &= ~mask;
	st |= flag;
}
inline void remove_state(unsigned &st, unsigned flag) {
	st &= ~flag;
}

inline bool is_power2(unsigned val)
{
	return (val&(val-1))==0;
}

// 返回大于等于val的最小的2的幂
unsigned next_power2(unsigned val);

// 返回2为底的对数,val必须为2的幂
uint32_t log2p2(uint32_t val);

}; // end of namespace wyc

#endif // WYC_HEADER_UTIL