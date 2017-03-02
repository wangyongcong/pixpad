#include "metric.h"
#include "log.h"

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
		m_timers.push_back(now);
		m_cur_timer_tid = tid;
	}

	void CSpwMetric::time_end()
	{
		auto end = std::chrono::steady_clock::now();
		auto beg = m_timers.back();
		m_timers.pop_back();
		auto dt = std::chrono::duration<float, std::milli>(end - beg).count();
		time_records.push_back({ m_cur_timer_tid, dt});
	}

	void CSpwMetric::report()
	{
		log_info("vertex count: %d", cnt_vertex);
		log_info("fragment count:%d", cnt_fragment);
		log_info("time used (ms):");
		for (auto &v : time_records)
		{
			log_info("  %d: %.2f", v.first, v.second);
		}
	}
}