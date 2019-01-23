#include <memory>
#include "spw_config.h"
#include "spw_tile_buffer.h"

namespace wyc {
	CSpwTileBuffer::CSpwTileBuffer()
		: m_buf(nullptr)
		, m_size(0)
		, m_width(0)
		, m_height(0)
	{
		
	}
	
	CSpwTileBuffer::~CSpwTileBuffer()
	{
		if(m_buf)
			delete [] m_buf;
	}
	
	bool CSpwTileBuffer::storage(unsigned int width, unsigned int height)
	{
		if(m_buf)
			release();
		m_size = width * height;
		m_buf = new char[m_size];
		m_width = width;
		m_height = height;
		return true;
	}
	
	void CSpwTileBuffer::release()
	{
		if(!m_buf)
			return;
		delete [] m_buf;
		m_buf = nullptr;
		m_size = 0;
		m_width = 0;
		m_height = 0;
	}
	
} // namespace wyc
