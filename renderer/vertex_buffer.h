#pragma once
#include <cstdint>
#include <iterator>
#include <cassert>

#include "OpenEXR/ImathVec.h"
#include "any_stride_iterator.h"
#include "vertex_layout.h"

namespace wyc
{
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
		CAttribArrayImpl(char *beg, char *end, unsigned stride)
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

		template<bool T>
		struct Iterator {};
		template<>
		struct Iterator<true> {
			typedef CAnyStrideIterator<float, CAnyReader> type;
		};
		template<>
		struct Iterator<false> {
			typedef CAnyStrideIterator<float, CAnyAccessor&> type;
		};
		using iterator = typename Iterator<IsConstant>::type;
	
		inline iterator begin()
		{
			return{ m_beg, m_stride };
		}
		inline iterator end()
		{
			return{ m_end };
		}
	protected:
		char *m_beg;
		char *m_end;
		unsigned m_stride;
	};

	typedef CAttribArrayImpl<false> CAttribArray;
	typedef CAttribArrayImpl<true> CConstAttribArray;

	class CVertexBuffer
	{
	public:
		CVertexBuffer();
		CVertexBuffer(const CVertexBuffer&) = delete;
		CVertexBuffer& operator = (const CVertexBuffer&) = delete;
		~CVertexBuffer();

		void resize(unsigned vertex_count);
		void clear();
		
		void set_attribute(EAttribUsage usage, uint8_t component);
		CAttribArray get_attribute(EAttribUsage usage);
		CConstAttribArray get_attribute(EAttribUsage usage) const;
		inline bool has_attribute(EAttribUsage usage) const {
			return m_attr_tbl[usage] != nullptr;
		}

		typedef CAnyStrideIterator<float, CAnyReader> const_iterator;
		typedef CAnyStrideIterator<float, CAnyAccessor&> iterator;
		inline iterator begin()
		{
			return{ m_data, m_vert_size };
		}
		inline iterator end()
		{
			return{ m_data + m_vert_size * m_vert_cnt };
		}
		inline const_iterator begin() const
		{
			return{ m_data, m_vert_size };
		}
		inline const_iterator end() const
		{
			return{ m_data + m_vert_size * m_vert_cnt };
		}

		class CStreamIterator : public CAnyStrideIterator<float>
		{
		public:
			CStreamIterator() : CAnyStrideIterator()
			{
			}
			CStreamIterator(void *beg, size_t stride = 0) 
				: CAnyStrideIterator(beg, stride)
			{
			}
			inline const float* operator * () const
			{
				return (float*)(m_cursor);
			}
		};
		inline CStreamIterator stream_begin() const
		{
			return{ m_data, m_vert_size };
		}
		inline CStreamIterator stream_end() const
		{
			return{ m_data + m_vert_size * m_vert_cnt };
		}

		inline const float* get_vertex_stream() const 
		{
			return reinterpret_cast<const float*>(m_data);
		}
		inline size_t size() const
		{
			return m_vert_cnt;
		}
		inline uint16_t vertex_size() const
		{
			return m_vert_size;
		}
		inline uint16_t vertex_component() const {
			return m_vert_componet;
		}

		inline const float* get_attrib_stream(EAttribUsage usage) const
		{
			assert(m_attr_tbl[usage]);
			return reinterpret_cast<const float*>(m_data + m_attr_tbl[usage]->offset);
		}
		inline size_t attrib_offset(EAttribUsage usage) const 
		{
			assert(m_attr_tbl[usage]);
			return m_attr_tbl[usage]->offset;
		}
		inline size_t attrib_component(EAttribUsage usage) const {
			assert(m_attr_tbl[usage]);
			return m_attr_tbl[usage]->component;
		}

	protected:
		char *m_data;
		size_t m_data_size;
		size_t m_vert_cnt;
		uint16_t m_vert_size;
		uint16_t m_vert_componet;
		VertexAttrib* m_attr_tbl[ATTR_MAX_COUNT];
	};


} // namespace wyc