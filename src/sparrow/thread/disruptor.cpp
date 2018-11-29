#include <emmintrin.h>
#include "disruptor.h"

int64_t disruptor::barrier::get_min()
{
	int64_t min_pos = 0x7fffffffffffffff;
	for (auto itr = _limit_seq.begin(); itr != _limit_seq.end(); ++itr)
	{
		auto itr_pos = (*itr)->pos().aquire();
		if (itr_pos < min_pos) min_pos = itr_pos;
	}
	return _last_min = min_pos;
}

int64_t disruptor::barrier::wait_for(int64_t pos)const
{
	if (_last_min >= pos)
		return _last_min;

	int64_t min_pos = 0x7fffffffffffffff;
	for (auto itr = _limit_seq.begin(); itr != _limit_seq.end(); ++itr)
	{
		int64_t itr_pos = 0;
		const sequence &seq = (*itr)->pos();
		itr_pos = seq.get();
		while (itr_pos < pos) {
			while (1) { // enter spin
				_mm_pause(); // pause, about 12ns
				if (seq.get() >= pos) break;
				// if no waiting threads, about 113ns
				// else lead to thread switching
				std::this_thread::yield();
				if (seq.get() >= pos) break;
			}
			itr_pos = seq.aquire();
		}

		if (seq.alert())
		{
			(*itr)->check_alert();
			if (itr_pos < pos)
				throw eof();
		}

		if (itr_pos < min_pos)
			min_pos = itr_pos;
	}
	assert(min_pos != 0x7fffffffffffffff);
	return _last_min = min_pos;
}
