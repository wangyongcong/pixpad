#pragma once
#include "platform_info.h"
#include <cstdlib>
#include <cassert>
#if defined(WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "spw_config.h"

namespace wyc
{
	PlatformInfo g_platform_info;

	PlatformInfo::PlatformInfo()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			architecture = "amd64";
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			architecture = "arm";
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			architecture = "ia64";
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			architecture = "intel";
			break;
		default:
			architecture = "unknown";
			break;
		}
		num_processor = si.dwNumberOfProcessors;
		page_size = si.dwPageSize;
		num_core = 0;
		memset(cacheline_size, 0, sizeof(cacheline_size));

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
						cacheline_size[buffer[i].Cache.Level - 1] = buffer[i].Cache.LineSize;
					break;
				case RelationProcessorCore:
					num_core += 1;
					break;
				default:
					break;
				}
			}
		}
		free(buffer);
		assert(cacheline_size[0] == CACHE_LINE_SIZE && "cache line size not match!");
	}

} // namespace wyc