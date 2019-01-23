#pragma once
#include "util.h"

namespace wyc
{
	class CSpwTileBuffer
	{
	public:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwTileBuffer)
		CSpwTileBuffer();
		~CSpwTileBuffer();
		
		bool storage(unsigned width, unsigned height);
		void release();
		
	private:
		char * m_buf;
		size_t m_size;
		unsigned m_width, m_height;
		
	}; // class CSpwTileBuffer
} // namespace wyc
