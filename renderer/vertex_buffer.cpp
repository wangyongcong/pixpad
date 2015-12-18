#include "vertex_buffer.h"

#include <cstring>

namespace wyc
{
	CVertexBuffer::CVertexBuffer() 
		: m_data(nullptr)
		, m_data_size(0)
		, m_vert_cnt(0)
		, m_vert_size(0)
	{
		memset(&m_attr_tbl, 0, sizeof(m_attr_tbl));
	}

	CVertexBuffer::~CVertexBuffer()
	{
		clear();
	}

	void CVertexBuffer::set_attribute(EAttribUsage usage, uint8_t element_count)
	{
		auto va = m_attr_tbl[usage];
		if (!va) {
			m_attr_tbl[usage] = new VertexAttrib{
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
		m_data_size = m_vert_size * vertex_count;
		m_data = new char[m_data_size];
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
		m_data_size = 0;
		m_vert_cnt = 0;
		m_vert_size = 0;
	}

	CAttribArray CVertexBuffer::get_attribute(EAttribUsage usage)
	{
		auto va = m_attr_tbl[usage];
		if (!va)
			return CAttribArray();
		auto beg = m_data + va->offset;
		auto end = beg + m_data_size;
		return CAttribArray(beg, end, m_vert_size);
	}

	CConstAttribArray CVertexBuffer::get_attribute(EAttribUsage usage) const
	{
		auto va = m_attr_tbl[usage];
		if (!va)
			return CConstAttribArray();
		auto beg = m_data + va->offset;
		auto end = beg + m_data_size;
		return CConstAttribArray(beg, end, m_vert_size);
	}
	
}