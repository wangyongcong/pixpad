#pragma once

#include <string>

namespace wyc
{
	struct PlatformInfo
	{
		PlatformInfo();
		std::string architecture;
		unsigned num_processor;
		unsigned num_core;
		unsigned page_size;
		unsigned cacheline_size[3];
	};

	extern PlatformInfo g_platform_info;

	inline unsigned page_size()
	{
		return g_platform_info.page_size;
	}

	inline unsigned cache_line_size()
	{
		return g_platform_info.cacheline_size[0];
	}

	inline unsigned core_num()
	{
		return g_platform_info.num_core;
	}

} // namespace wyc