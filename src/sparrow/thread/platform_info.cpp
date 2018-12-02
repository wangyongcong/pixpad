#include "platform_info.h"
#include <cstdlib>
#include <cassert>
#include <thread>
#if defined(WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#include "spw_config.h"

namespace wyc
{
#ifdef __APPLE__
	bool get_sys_string(int ctl_type, int ctl_name, std::string &ret)
	{
		int mib[2] = {ctl_type, ctl_name};
		size_t len = 0;
		char* data = nullptr;
		int code = sysctl(mib, 2, NULL, &len, NULL, 0);
		if(code)
			return false;
		data = new char[len];
		code = sysctl(mib, 2, data, &len, NULL, 0);
		if(!code)
			ret = data;
		delete [] data;
		return code == 0;
	}
	
	template<class T>
	bool get_sys_var(int ctl_type, int ctl_name, T &ret)
	{
		int mib[2] = {ctl_type, ctl_name};
		size_t len = sizeof(T);
		int code = sysctl(mib, 2, &ret, &len, NULL, 0);
		return code == 0;
	}
#endif // __APPLE__
	
	PlatformInfo::PlatformInfo()
		: os("Unknown")
		, architecture("Unknown")
		, ncpu(1)
		, cpu_freq(0)
		, cacheline(64)
		, cache_size({64, 64, 64})
		, memory(0)
		, page_size(4 * 1024)
	{
#if defined(WIN32) || defined(WIN64)
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
		cpu_count = si.dwNumberOfProcessors;
		page_size = si.dwPageSize;
		relation_core = 0;
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
					if(i == 0)
						cacheline = buffer[i].Cache.LineSize;
					if(buffer[i].Cache.Level <= 3)
						cacheline_size[buffer[i].Cache.Level - 1] = buffer[i].Cache.Size;
					break;
				case RelationProcessorCore:
					relation_core += 1;
					break;
				default:
					break;
				}
			}
		}
		free(buffer);
		assert(cacheline_size[0] == CACHE_LINE_SIZE && "cache line size not match!");
#elif defined(__APPLE__)
		std::string s1;
		int val;
		uint64_t val64;
		if(get_sys_string(CTL_KERN, KERN_OSTYPE, os)
			&& get_sys_string(CTL_KERN, KERN_OSRELEASE, s1))
		{
			os += " ";
			os += s1;
		}
		get_sys_string(CTL_HW, HW_MACHINE, architecture);
		get_sys_var(CTL_HW, HW_NCPU, ncpu);
		get_sys_var(CTL_HW, HW_CPU_FREQ, cpu_freq);
		if(get_sys_var(CTL_HW, HW_CACHELINE, val))
			cacheline = (size_t)val;
		if(get_sys_var(CTL_HW, HW_L1DCACHESIZE, val))
			cache_size[0] = (size_t)val;
		if(get_sys_var(CTL_HW, HW_L2CACHESIZE, val))
			cache_size[1] = (size_t)val;
		if(get_sys_var(CTL_HW, HW_L3CACHESIZE, val))
			cache_size[2] = (size_t)val;
		if(get_sys_var(CTL_HW, HW_PAGESIZE, val))
			page_size = (size_t)val;
		if(get_sys_var(CTL_HW, HW_MEMSIZE, val64))
			memory = val64;
#endif
	}
} // namespace wyc
