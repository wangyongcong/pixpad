#pragma once
#include <vector>
#include <stack>
#include <chrono>
#include <string>
#include "engine.h"

namespace wyc
{
	enum SPW_TIMER
	{
		VERTEX_SHADER = 0,
		PIXEL_SHADER,
		DRAW_TRIANGLE,
		
		SPW_TIMER_COUNT
	};
	
	enum SPW_COUNTER
	{
		VERTEX_COUNT = 0,
		PIXEL_COUNT,
		TRIANGLE_COUNT,
		VIEWPORT_CULLING_COUNT,
		BACKFACE_CULLING_COUNT,
		DEPTH_CULLING_COUNT,
		
		SPW_COUNTER_COUNT
	};
	
	class WYCAPI CSpwMetric
	{
	public:
		static CSpwMetric* singleton() {
			static CSpwMetric ls_metric;
			return &ls_metric;
		}
		void clear();
		void time_beg(SPW_TIMER tid);
		void time_end();
		inline void count(SPW_COUNTER tid) {
			m_counters[tid] += 1;
		}
		void report();
		
	private:
		CSpwMetric();
		typedef std::chrono::steady_clock::time_point time_point_t;
		std::vector<std::pair<unsigned, time_point_t>> m_timer_stack;
		std::vector<float> m_timers;
		std::vector<unsigned> m_counters;
	};
	
	class WYCAPI CSpwMetricTimer
	{
	public:
		CSpwMetricTimer(SPW_TIMER tid) {
			CSpwMetric::singleton()->time_beg(tid);
		}
		~CSpwMetricTimer() {
			CSpwMetric::singleton()->time_end();
		}
	};
	
	class WYCAPI CSimpleTimer
	{
	public:
		CSimpleTimer(const char *name, bool start=true);
		~CSimpleTimer();
		void pause();
		void resume();
		
	private:
		std::string m_name;
		typedef std::chrono::steady_clock clock_t;
		typedef clock_t::time_point time_point_t;
		time_point_t m_begin;
		float m_result;
		bool m_is_paused;
	};
} // namespace wyc

#ifdef NO_PERF

// timer
#define TIME_VERTEX_SHADER
#define TIME_PIXEL_SHADER
#define TIME_DRAW_TRIANGLE

// counter
#define COUNT_VERTEX
#define COUNT_PIXEL
#define COUNT_TRIANGLE
#define VIEWPORT_CULLING
#define BACKFACE_CULLING
#define DEPTH_CULLING

#else // !NO_PERF

#define _NEW_TIMER(name) wyc::CSpwMetricTimer __TIMER_##name##__(wyc::SPW_TIMER::name);
#define _INC_COUNTER(name) {wyc::CSpwMetric::singleton()->count(name);}

// timer
#define TIME_VERTEX_SHADER _NEW_TIMER(VERTEX_SHADER)
#define TIME_PIXEL_SHADER _NEW_TIMER(PIXEL_SHADER)
#define TIME_DRAW_TRIANGLE _NEW_TIMER(DRAW_TRIANGLE)

// counter
#define COUNT_VERTEX _INC_COUNTER(VERTEX_COUNT)
#define COUNT_PIXEL _INC_COUNTER(PIXEL_COUNT)
#define COUNT_TRIANGLE _INC_COUNTER(TRIANGLE_COUNT)
#define VIEWPORT_CULLING _INC_COUNTER(VIEWPORT_CULLING_COUNT)
#define BACKFACE_CULLING _INC_COUNTER(BACKFACE_CULLING_COUNT)
#define DEPTH_CULLING _INC_COUNTER(DEPTH_CULLING_COUNT)

#endif // NO_PEF

#define TIMER(name)  CSimpleTimer __SIMPER_TIMER_##name##__(#name)
