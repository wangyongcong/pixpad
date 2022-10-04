#include "vertex_buffer.h"
#include <cstring>
#include <bitset>
#include "common/memory.h"
#include "common/log_macros.h"

namespace wyc
{
	CVertexBuffer::CVertexBuffer() 
		: m_data(nullptr)
		, m_data_size(0)
		, m_vert_count(0)
		, m_vert_size(0)
		, m_is_sealed(false)
		, m_is_float(false)
		, m_attr_mask(0)
		, m_attr_list(nullptr)
		, m_attr_list_tail(&m_attr_list)
	{
	}

	CVertexBuffer::~CVertexBuffer()
	{
		clear();
	}

	void CVertexBuffer::set_attribute(EAttributeUsage usage, TinyImageFormat format, uint8_t stream_index)
	{
		if(m_is_sealed)
		{
			log_error("Vertex layout is sealed");
			return;
		}
		bool has_attr = m_attr_mask[(int)usage];
		if(!has_attr)
		{
			VertexAttributeLinkedNode *va = wyc_new(VertexAttributeLinkedNode, usage, format, stream_index);
			va->linked_next = nullptr;
			*m_attr_list_tail = va;
			m_attr_list_tail = &va->linked_next;
		}
		else
		{
			for(VertexAttributeLinkedNode *va = m_attr_list; va != nullptr; va = va->linked_next)
			{
				if(va->usage == usage)
				{
					va->set_format(format);
					va->stream_index = stream_index;
					break;
				}
			}
		}
}

	void CVertexBuffer::resize(unsigned vertex_count)
	{
		if(!m_is_sealed)
		{
			m_is_float = true;
			std::bitset<256> array_mask;
			for(VertexAttributeLinkedNode *va = m_attr_list; va != nullptr; va = va->linked_next)
			{
				array_mask.set(va->stream_index, true);
				m_is_float &= va->is_float;
			}
			m_attr_stream.clear();
			m_attr_stream.resize(array_mask.count());

			m_vert_size = 0;
			for(VertexAttributeLinkedNode *va = m_attr_list; va != nullptr; va = va->linked_next)
			{
				VertexAttribteStream &va_array = m_attr_stream[va->stream_index];
				*va_array.attr_list_tail = va;
				va_array.attr_list_tail = &va->stream_next;
				va_array.attr_count += 1;
				va->offset = va_array.stride;
				va_array.stride += va->size;
				m_vert_size += va->size;
			}

			m_is_sealed = true;
		}

		if(m_data)
		{
			wyc_free(m_data);
			m_data = nullptr;
		}
		m_vert_count = vertex_count;
		if(vertex_count > 0)
		{
			m_data_size = vertex_count * m_vert_size;
			m_data = (char*)wyc_malloc(m_data_size);

			size_t data_offset = 0;
			for(VertexAttribteStream &va_array: m_attr_stream)
			{
				va_array.data = m_data + data_offset;
				va_array.size = va_array.stride * vertex_count;
				data_offset += va_array.size;
			}
			assert(data_offset == m_data_size);
		}
		else
		{
			m_data_size = 0;
			for(VertexAttribteStream &va_array: m_attr_stream)
			{
				va_array.data = nullptr;
				va_array.size = 0;
			}
		}
	}

	void CVertexBuffer::clear()
	{
		if (m_data)
		{
			wyc_free(m_data);
			m_data = nullptr;
			m_data_size = 0;
			m_vert_count = 0;
		}
		for(VertexAttributeLinkedNode *va = m_attr_list; va != nullptr;)
		{
			auto to_del = va;
			va = va->linked_next;
			wyc_delete(to_del);
		}
		m_attr_list = nullptr;
		m_attr_list_tail = &m_attr_list;
		m_vert_size = 0;
		m_is_sealed = false;
		m_is_float = false;
		m_attr_stream.clear();
	}

	CAttribArray CVertexBuffer::get_attribute(EAttributeUsage usage)
	{
		if(m_data)
		{
			auto va = find_attribute_impl(usage);
			if(va)
			{
				const VertexAttribteStream& va_array = m_attr_stream[va->stream_index];
				auto beg = va_array.data + va->offset;
				auto end = beg + m_vert_count * va_array.stride;
				return CAttribArray(beg, end, va_array.stride);
			}
		}
		return CAttribArray();
	}

	CConstAttribArray CVertexBuffer::get_attribute(EAttributeUsage usage) const
	{
		if(m_data)
		{
			auto va = find_attribute_impl(usage);
			if(va)
			{
				const VertexAttribteStream& va_array = m_attr_stream[va->stream_index];
				auto beg = va_array.data + va->offset;
				auto end = beg + m_vert_count * va_array.stride;
				return CConstAttribArray(beg, end, va_array.stride);
			}
		}
		return CConstAttribArray();
	}

	const CVertexBuffer::VertexAttributeLinkedNode* CVertexBuffer::find_attribute_impl(EAttributeUsage usage) const
	{
		if(!m_attr_mask[(int)usage])
		{
			return nullptr;
		}
		for(const VertexAttributeLinkedNode *va = m_attr_list; va != nullptr; va = va->linked_next)
		{
			if(va->usage == usage)
			{
				return va;
			}
		}
		return nullptr;
	}
}
