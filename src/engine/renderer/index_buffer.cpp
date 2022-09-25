#include "index_buffer.h"

namespace wyc
{
	CIndexBuffer::CIndexBuffer()
		: m_data(nullptr)
		, m_data_size(0)
		, m_count(0)
		, m_stride(0)
		, m_max_val(0)
	{
	}

	CIndexBuffer::~CIndexBuffer()
	{
		if (m_data) {
			delete[] m_data;
			m_data = nullptr;
		}
	}
	
	void CIndexBuffer::clear()
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}
		m_data_size = 0;
		m_count = 0;
		m_stride = 0;
	}

	void CIndexBuffer::_resize(size_t count, uint8_t stride)
	{
		if (m_data)
			clear();
		size_t sz = stride * count;
		if (!sz)
			return;
		m_data = new char[sz];
		m_data_size = sz;
		m_count = count;
		m_stride = stride;
	}

} // namespace wyc