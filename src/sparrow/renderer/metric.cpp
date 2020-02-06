#include "metric.h"
#include "stb_log.h"

#define my_counter(name) m_counters[int(name)]
#define my_timer(name) m_timers[int(name)]

namespace wyc
{
	CSpwMetric::CSpwMetric()
	{
		m_timer_stack.reserve(64);
		m_timers.resize(SPW_TIMER_COUNT, 0.0f);
		m_counters.resize(SPW_COUNTER_COUNT, 0);
	}

	void CSpwMetric::clear()
	{
		m_timers.resize(SPW_TIMER_COUNT, 0.0f);
		m_counters.resize(SPW_COUNTER_COUNT, 0);
	}
	
	void CSpwMetric::time_beg(SPW_TIMER tid)
	{
		auto now = std::chrono::steady_clock::now();
		m_timer_stack.push_back({ tid, now });
	}

	void CSpwMetric::time_end()
	{
		auto end = std::chrono::steady_clock::now();
		auto &t = m_timer_stack.back();
		auto dt = std::chrono::duration<float, std::milli>(end - t.second).count();
		m_timers[t.first] += dt;
		m_timer_stack.pop_back();
	}
	
	void CSpwMetric::report()
	{
		static const std::string splitter(32, '-');
		log_info(splitter);
		log_info("| viewport culling: %d", my_counter(VIEWPORT_CULLING_COUNT));
		log_info("| backface culling: %d", my_counter(BACKFACE_CULLING_COUNT));
		log_info("| depth culling: %d", my_counter(DEPTH_CULLING_COUNT));
		log_info("| triangles count: %d", my_counter(TRIANGLE_COUNT));
		log_info("| vertex count: %d", my_counter(VERTEX_COUNT));
		log_info("| pixel count: %d", my_counter(PIXEL_COUNT));
		log_info("| time used by vs: %.2f ms", my_timer(VERTEX_SHADER));
		log_info("| time used by ps: %.2f ms", my_timer(PIXEL_SHADER));
		log_info("| time used by draw: %.2f ms", my_timer(DRAW_TRIANGLE));
		log_info(splitter);
	}
	
	CSimpleTimer::CSimpleTimer(const char *name, bool start)
	: m_name(name)
	, m_begin()
	, m_result(0)
	, m_is_paused(!start)
	{
		if(start)
			m_begin = clock_t::now();
	}
	
	CSimpleTimer::~CSimpleTimer() {
		pause();
		log_info("Time used for [%s]: %f ms", m_name, m_result);
	}
	
	void CSimpleTimer::pause()
	{
		if(m_is_paused) return;
		auto end = clock_t::now();
		auto dt = std::chrono::duration<float, std::milli>(end - m_begin).count();
		m_result += dt;
		m_is_paused = true;
	}
	
	void CSimpleTimer::resume()
	{
		if(!m_is_paused) return;
		m_is_paused = false;
		m_begin = clock_t::now();
	}
}
