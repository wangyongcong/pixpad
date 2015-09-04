#pragma once

#include <string>

namespace wyc
{
	struct platform_info
	{
		platform_info();
		std::string m_architecture;
		size_t m_num_processor;
		size_t m_num_core;
		size_t m_page_size;
		size_t m_cacheline_size[3];
	};

	extern platform_info g_platform_info;

	inline size_t page_size()
	{
		return g_platform_info.m_page_size;
	}

	inline size_t cache_line_size()
	{
		return g_platform_info.m_cacheline_size[0];
	}

	inline size_t core_num()
	{
		return g_platform_info.m_num_core;
	}

#define CACHE_LINE_SIZE 64

#define PAGE_SIZE 4096

} // namespace wyc