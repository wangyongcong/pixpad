#include "metric.h"
#include "stb_log.h"

namespace wyc
{
	CSpwMetric::CSpwMetric()
		: cnt_vertex(0)
		, cnt_fragment(0)
	{
		m_timers.reserve(64);
		time_records.reserve(64);
	}

	void CSpwMetric::time_beg(unsigned tid)
	{
		auto now = std::chrono::steady_clock::now();
		m_timers.push_back({ tid, now });
	}

	void CSpwMetric::time_end()
	{
		auto end = std::chrono::steady_clock::now();
		auto &t = m_timers.back();
		auto dt = std::chrono::duration<float, std::milli>(end - t.second).count();
		time_records.push_back({ t.first, dt});
		m_timers.pop_back();
	}

	void CSpwMetric::report()
	{
		log_info("vertex count: %d", cnt_vertex);
		log_info("fragment count: %d", cnt_fragment);
		log_info("time used (ms):");
		for (auto &v : time_records)
		{
			log_info("  %d: %.2f", v.first, v.second);
		}
	}
}
