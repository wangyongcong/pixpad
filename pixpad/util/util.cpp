#include "util.h"

namespace wyc
{

unsigned next_power2(unsigned val)
{
	// val可能是2的幂
	--val;
	// 将MSB右边的所有位全部置为1
	val |= (val >>  1);
	val |= (val >>  2);
	val |= (val >>  4);
	val |= (val >>  8);		/* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xffff)
	val |= (val >> 16);		/* For 32 bit int systems */
#if (UINT_MAX > 0xffffffffUL)
	val |= (val >> 32);		/* For 64 bit int systems */
#endif // UINT_MAX != 0xffff
#endif // UINT_MAX > 0xffffffffUL
	++val;
	return val;
}

uint32_t log2p2(uint32_t val)
{
	// 32位DeBruijn数列
	static const int ls_DeBruijn32[32] =
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return ls_DeBruijn32[(uint32_t)(val * 0x077CB531U) >> 27];
}

bool wstr2str(std::string &dst, const std::wstring &src)
{
	size_t src_size = src.size() * sizeof(wchar_t);
	if (src_size >= 1024)
		return false;
	size_t dst_size = src_size + 1;
	char *pdst = new char[dst_size];
	size_t cnt; // chars written into buffer including null terminated
	errno_t err = wcstombs_s(&cnt, pdst, dst_size, src.c_str(), src_size);
	if (err)
		return false;
	dst.assign(pdst, cnt-1);
	delete[] pdst;
	return true;
}

}; // end of namespace wyc