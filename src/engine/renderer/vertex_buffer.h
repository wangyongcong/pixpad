#pragma once
#include <cstdint>
#include <iterator>
#include <cassert>
#include <bitset>
#include <ImathVec.h>
#include "engine.h"
#include "common/any_stride_iterator.h"
#include "common/common_macros.h"
#include "vertex_layout.h"

namespace wyc
{
	template<bool T>
	struct CAttribArrayIterator {};
	template<>
	struct CAttribArrayIterator<true> {
		typedef CAnyStrideIterator<float, CAnyReader> type;
	};
	template<>
	struct CAttribArrayIterator<false> {
		typedef CAnyStrideIterator<float, CAnyAccessor&> type;
	};
	
	template<bool IsConstant>
	class CAttribArrayImpl
	{
		typedef CAttribArrayImpl MyType;
	public:
		CAttribArrayImpl()
			: m_beg(nullptr)
			, m_end(nullptr)
			, m_stride(0)
		{}
		CAttribArrayImpl(uint8_t *beg, uint8_t *end, unsigned stride)
			: m_beg(beg)
			, m_end(end)
			, m_stride(stride)
		{
			assert(m_end - m_beg >= long(m_stride));
		}
		CAttribArrayImpl(const MyType& other)
		{
			*this = other;
		}
		MyType& operator = (const MyType& other)
		{
			m_beg = other.m_beg;
			m_end = other.m_end;
			m_stride = other.m_stride;
			return *this;
		}

		operator bool() const
		{
			return m_beg < m_end;
		}

		using iterator = typename CAttribArrayIterator<IsConstant>::type;
	
		inline iterator begin()
		{
			return{ m_beg, m_stride };
		}
		inline iterator end()
		{
			return{ m_end };
		}
	protected:
		uint8_t *m_beg;
		uint8_t *m_end;
		unsigned m_stride;
	};

	typedef CAttribArrayImpl<false> CAttribArray;
	typedef CAttribArrayImpl<true> CConstAttribArray;

	class WYCAPI VertexBuffer
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(VertexBuffer);
	public:
		VertexBuffer();
		~VertexBuffer();
		/**
		 * \brief Clear vertex layout and data.
		 */
		void clear();
		/**
		 * \brief Resize vertex buffer. Seal vertex layout and allocate memory for vertex data.
		 * \param vertex_count Count of vertex
		 */
		void resize(unsigned vertex_count);
		/**
		 * \brief Setup vertex layout
		 * \param usage Vertex attribute usage
		 * \param format Attribute data format
		 * \param stream_index Attribute data is store in which array. Data may be save as AOS or SOA
		 */
		void set_attribute(EAttributeUsage usage, TinyImageFormat format, uint8_t stream_index=0);
		void set_attribute(EAttributeUsage usage, uint8_t channel, uint8_t size, bool is_float, uint8_t stream_index=0);

		CAttribArray get_attribute(EAttributeUsage usage);
		CConstAttribArray get_attribute(EAttributeUsage usage) const;

		bool has_attribute(EAttributeUsage usage) const
		{
			return m_attr_mask[(int)usage];
		}

		typedef CAnyStrideIterator<float, CAnyReader> const_iterator;
		typedef CAnyStrideIterator<float, CAnyAccessor&> iterator;

		iterator begin()
		{
			return{ m_data, m_vert_size };
		}

		iterator end()
		{
			return{ m_data + m_data_size };
		}

		const_iterator begin() const
		{
			return{ m_data, m_vert_size };
		}

		const_iterator end() const
		{
			return{ m_data + m_data_size };
		}

		void* data()
		{
			return m_data;
		}

		const void* data() const
		{
			return m_data;
		}

		size_t data_size() const
		{
			return m_data_size;
		}

		float* get_buffer() 
		{
			return reinterpret_cast<float*>(m_data);
		}

		const float* get_buffer() const 
		{
			return reinterpret_cast<const float*>(m_data);
		}

		size_t size() const
		{
			return m_vert_count;
		}

		uint16_t vertex_size() const
		{
			return m_vert_size;
		}

		uint16_t vertex_component() const 
		{
			assert(m_is_float && "Vertex is not float component");
			return m_vert_size / sizeof(float);
		}

		void* attrib_stream(EAttributeUsage usage)
		{
			auto va = find_attribute_impl(usage);
			if (!va) return nullptr;
			auto& va_array = m_attr_stream[va->stream_index];
			return va_array.data + va->offset;
		}

		const void* attrib_stream(EAttributeUsage usage) const
		{
			auto va = find_attribute_impl(usage);
			if (!va) return nullptr;
			auto& va_array = m_attr_stream[va->stream_index];
			return va_array.data + va->offset;
		}

		size_t attrib_stride(EAttributeUsage usage) const 
		{
			auto va = find_attribute_impl(usage);
			return va ? va->size : 0;
		}

		size_t attrib_offset(EAttributeUsage usage) const
		{
			auto va = find_attribute_impl(usage);
			return va ? va->offset : 0;
		}

		size_t attrib_component(EAttributeUsage usage) const
		{
			auto va = find_attribute_impl(usage);
			return (va && va->is_float) ? va->channel : 0;
		}

		const VertexAttribute* find_attribute(EAttributeUsage usage) const
		{
			return find_attribute_impl(usage);
		}

	protected:
		uint8_t *m_data;
		size_t m_data_size;
		size_t m_vert_count;
		// vertex size in byte
		uint16_t m_vert_size;
		// attribute layout is setup
		bool m_is_sealed;
		// if all attributes are float data
		bool m_is_float;

		struct VertexAttributeLinkedNode : VertexAttribute
		{
			VertexAttributeLinkedNode* linked_next;
			VertexAttributeLinkedNode* stream_next;

			VertexAttributeLinkedNode(EAttributeUsage usage, TinyImageFormat format, uint8_t stream_index)
				: VertexAttribute(usage, format, stream_index)
				, linked_next(nullptr), stream_next(nullptr)
			{
			}
		};
		// attribute linked list
		std::bitset<256> m_attr_mask;
		VertexAttributeLinkedNode* m_attr_list;
		VertexAttributeLinkedNode** m_attr_list_tail;

		struct VertexAttribteStream
		{
			uint8_t* data;
			size_t size;
			// attribute stride in byte
			unsigned stride; 
			// attribute linked list
			unsigned attr_count;
			VertexAttributeLinkedNode* attr_list;
			VertexAttributeLinkedNode** attr_list_tail;

			VertexAttribteStream()
				: data(nullptr), size(0), stride(0), attr_count(0)
				, attr_list(nullptr), attr_list_tail(&attr_list)
			{
			}
		};
		// attribute layout
		std::vector<VertexAttribteStream> m_attr_stream;

		const VertexAttributeLinkedNode* find_attribute_impl(EAttributeUsage usage) const;
	};


} // namespace wyc
