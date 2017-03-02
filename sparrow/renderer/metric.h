#pragma once
#include <vector>
#include <stack>
#include <chrono>

namespace wyc
{
	class CSpwMetric
	{
	public:
		static CSpwMetric* singleton() {
			static CSpwMetric ls_metric;
			return &ls_metric;
		}
		unsigned cnt_vertex;
		unsigned cnt_fragment;
		std::vector<std::pair<unsigned, float>> time_records;
		void time_beg(unsigned tid);
		void time_end();
		void report();
	
	private:		
		CSpwMetric();
		unsigned m_cur_timer_tid;
		typedef std::chrono::steady_clock::time_point time_point_t;
		std::vector<time_point_t> m_timers;

	};

} // namespace wyc

#define COUNT_VERTEX {wyc::CSpwMetric::singleton()->cnt_vertex += 1;}
#define COUNT_FRAGMENT {wyc::CSpwMetric::singleton()->cnt_fragment += 1;}
#define TIMER_BEG(tag) {wyc::CSpwMetric::singleton()->time_beg(tag);}
#define TIMER_END {wyc::CSpwMetric::singleton()->time_end();}