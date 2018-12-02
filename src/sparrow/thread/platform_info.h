#pragma once
#include <array>
#include <string>

namespace wyc
{
	struct PlatformInfo
	{
		PlatformInfo();
		// OS name and version
		std::string os;
		// machine architecture
		std::string architecture;
		// number of cpus
		int ncpu;
		// cache line size in bytes
		size_t cacheline;
		// L1/L2/L3 cache size in bytes
		std::array<size_t, 3> cache_size;
		// physical memory size
		size_t memory;
		// software page size
		size_t page_size;
	};

	inline const PlatformInfo& get_platform_info() {
		static PlatformInfo s_platform_info;
		return s_platform_info;
	}

} // namespace wyc
