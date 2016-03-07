#pragma once

#include <cstdint>
#include <numeric>
#include <type_traits>
#include <common/any_stride_iterator.h>

namespace wyc
{
	class CIndexBuffer
	{
	public:
		CIndexBuffer();
		~CIndexBuffer();
		template<typename IndexType>
		void resize(size_t count);
		void clear();
		size_t size() const;
		uint8_t stride() const;
		
		using iterator = CAnyStrideIterator<unsigned>;
		inline iterator begin() {
			return{ m_data, m_stride };
		}
		inline iterator end() {
			return{ m_data + m_data_size };
		}

	protected:
		void _resize(size_t count, uint8_t stride);

		char *m_data;
		size_t m_data_size;
		size_t m_count;
		uint8_t m_stride;
		unsigned long m_max_val;
	};

	inline size_t CIndexBuffer::size() const
	{
		return m_count;
	}

	inline unsigned char CIndexBuffer::stride() const
	{
		return m_stride;
	}

	template<typename IndexType>
	inline void CIndexBuffer::resize(size_t count)
	{
		static_assert(std::is_integral<IndexType>(), "Index type is not integral");
		_resize(count, sizeof(IndexType));
		m_max_val = (unsigned long)std::numeric_limits<IndexType>::max();
	}

} // namespace wyc