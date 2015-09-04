#pragma once

#include "platform_info.h"

#include <cstdlib>

#if defined(WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace wyc
{
	platform_info g_platform_info;

	platform_info::platform_info()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			m_architecture = "amd64";
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			m_architecture = "arm";
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			m_architecture = "ia64";
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			m_architecture = "intel";
			break;
		default:
			m_architecture = "unknown";
			break;
		}
		m_num_processor = si.dwNumberOfProcessors;
		m_page_size = si.dwPageSize;
		m_num_core = 0;
		memset(m_cacheline_size, 0, sizeof(m_cacheline_size));

		SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;
		DWORD buffer_size = 0;
		GetLogicalProcessorInformation(0, &buffer_size);
		buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
		if (GetLogicalProcessorInformation(&buffer[0], &buffer_size))
		{
			for (size_t i = 0, offset = 0; offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) < buffer_size; offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION), ++i)
			{
				switch (buffer[i].Relationship)
				{
				case RelationCache:
					if(buffer[i].Cache.Level <= 3)
						m_cacheline_size[buffer[i].Cache.Level - 1] = buffer[i].Cache.LineSize;
					break;
				case RelationProcessorCore:
					m_num_core += 1;
					break;
				default:
					break;
				}
			}
		}
		free(buffer);

		if (m_cacheline_size[0] != CACHE_LINE_SIZE)
		{
			::OutputDebugString(L"\
------------------------------------------\n\
 [WARNING] Cache line size is not matched \n\
------------------------------------------\n");
		}

		if (m_page_size != PAGE_SIZE)
		{
			::OutputDebugString(L"\
------------------------------------\n\
 [WARNING] Page size is not matched \n\
------------------------------------\n");
		}
	}

} // namespace wyc