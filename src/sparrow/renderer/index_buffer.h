#pragma once
#include <cstdint>
#include <numeric>
#include <vector>
#include <type_traits>
#include "any_stride_iterator.h"

namespace wyc
{
	template<typename IndexType>
	class CIndexBufferT
	{
	public:
		static_assert(std::is_integral<IndexType>::value, "Index type is not integral");
		CIndexBufferT()
			: m_data(nullptr)
			, m_data_size(0)
			, m_count(0)
			, m_stride(0)
			, m_max_val(0)
		{
		}

		~CIndexBufferT()
		{
			if (m_data) {
				delete[] m_data;
				m_data = nullptr;
			}
		}

		void resize(size_t count) {
			if (m_data)
				clear();
			size_t sz = sizeof(IndexType) * count;
			if (!sz)
				return;
			m_data = new char[sz];
			m_data_size = sz;
			m_count = count;
			m_stride = sizeof(IndexType);
			m_max_val = (unsigned long)std::numeric_limits<IndexType>::max();
		}

		void clear()
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

		size_t size() const {
			return m_count;
		}

		uint8_t stride() const {
			return m_stride;
		}

		inline const char* get_index_stream() const {
			return m_data;
		}

		using const_iterator = CAnyStrideIterator<unsigned, CAnyReader>;
		using iterator = CAnyStrideIterator<unsigned, CAnyAccessor&>;
		inline iterator begin() {
			return{ m_data, m_stride };
		}
		inline iterator end() {
			return{ m_data + m_data_size };
		}

	protected:
		char *m_data;
		size_t m_data_size;
		size_t m_count;
		uint8_t m_stride;
		unsigned long m_max_val;
	};

	//typedef CIndexBufferT<uint32_t> CIndexBuffer;
	typedef std::vector<uint32_t> CIndexBuffer;

} // namespace wyc