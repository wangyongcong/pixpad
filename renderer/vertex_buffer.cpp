#include "vertex_buffer.h"

#include <cstring>

namespace wyc
{
	CVertexBuffer::CVertexBuffer() : m_data(nullptr)
	{
		memset(&m_attr_tbl, 0, sizeof(m_attr_tbl));
	}

	CVertexBuffer::~CVertexBuffer()
	{
		clear();
	}

	void CVertexBuffer::set_attribute(EAttributeUsage usage, uint8_t element_count)
	{
		auto va = m_attr_tbl[usage];
		if (!va) {
			m_attr_tbl[usage] = new VertexAttribute{
				usage, element_count, 0
			};
		}
		else {
			va->elem_cnt = element_count;
		}
	}

	void CVertexBuffer::resize(unsigned vertex_count)
	{
		m_vert_size = 0;
		for (auto va : m_attr_tbl)
		{
			if (!va) continue;
			va->offset = m_vert_size;
			m_vert_size += va->elem_cnt * sizeof(float);
		}
		if (m_data)
		{
			delete[] m_data;
		}
		m_data = new char[m_vert_size * vertex_count];
		m_vert_cnt = vertex_count;
	}

	void CVertexBuffer::clear()
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}
		for (auto &va : m_attr_tbl)
		{
			if (!va) continue;
			delete va;
			va = nullptr;
		}
	}

	CAttributeArray CVertexBuffer::get_attribute(EAttributeUsage usage)
	{
		auto va = m_attr_tbl[usage];
		if (!va)
			return CAttributeArray();
		auto beg = m_data + va->offset;
		auto end = beg + m_vert_size * m_vert_cnt;
		return CAttributeArray(beg, end, m_vert_size);
	}

	
}