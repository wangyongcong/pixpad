#pragma once
#include <cstdint>
#include <numeric>
#include <vector>
#include "common/memory.h"
#include "common/util_macros.h"

namespace wyc
{
	class IndexBuffer
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(IndexBuffer)
	public:
		IndexBuffer()
			: m_data(nullptr)
			, m_data_size(0)
			, m_count(0)
			, m_max_index(std::numeric_limits<uint32_t>::max())
			, m_stride(sizeof(uint32_t))
		{
		}

		~IndexBuffer()
		{
			if (m_data) {
				wyc_free(m_data);
				m_data = nullptr;
			}
		}

		void resize(size_t count, size_t max_index=std::numeric_limits<uint32_t>::max())
		{
			if (m_data)
			{
				clear();
			}
			m_stride = 0;
			if(max_index <= std::numeric_limits<uint16_t>::max())
			{
				m_stride = sizeof(uint16_t);
				m_max_index = std::numeric_limits<uint16_t>::max();
			}
			else if(max_index <= std::numeric_limits<uint32_t>::max())
			{
				m_stride = sizeof(uint32_t);
				m_max_index = std::numeric_limits<uint32_t>::max();
			}
			else
			{
				assert(0 && "max index value should be inside uint32_t");
				return;
			}
			m_count = count;
			m_data_size = m_stride * count;
			if(m_data_size > 0)
			{
				m_data = (char*)wyc_malloc(m_data_size);
			}
		}

		void clear()
		{
			if (m_data)
			{
				wyc_free(m_data);
				m_data = nullptr;
				m_data_size = 0;
			}
			m_count = 0;
		}

		size_t size() const
		{
			return m_count;
		}

		uint8_t stride() const
		{
			return m_stride;
		}

		bool is_short() const
		{
			return m_stride == sizeof(uint16_t);
		}

		size_t data_size() const
		{
			return m_data_size;
		}

		char* data()
		{
			return m_data;
		}

		const char* data() const
		{
			return m_data;
		}

		template<class T>
		T* data()
		{
			static_assert(std::is_integral_v<T>, "Index type is not integral");
			assert(sizeof(T) == m_stride);
			return (T*)m_data;
		}

		template<class T>
		const T* data() const
		{
			static_assert(std::is_integral_v<T>, "Index type is not integral");
			assert(sizeof(T) == m_stride);
			return (T*)m_data;
		}

		bool is_valid(void* p) const
		{
			return ((char*)p >= m_data) && ((char*)p < m_data + m_data_size);
		}

		bool is_valid_end(void* p) const
		{
			return (char*)p == (m_data + m_data_size);
		}

	protected:
		char *m_data;
		size_t m_data_size;
		size_t m_count;
		size_t m_max_index;
		uint8_t m_stride;
	};
} // namespace wyc