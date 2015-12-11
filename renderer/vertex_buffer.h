#pragma once
#include <cstdint>
#include <iterator>
#include <cassert>

#include "OpenEXR/ImathVec.h"

namespace wyc
{
	enum EAttributeUsage
	{
		ATTR_POSITION = 0,
		ATTR_COLOR,
		ATTR_TEXTURE,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_USAGE_0,
		ATTR_USAGE_1,
		ATTR_USAGE_2,
		ATTR_USAGE_3,
		ATTR_MAX_COUNT,
	};

	struct VertexAttribute
	{
		EAttributeUsage usage;
		uint8_t elem_cnt;
		uint32_t offset;
	};

	struct VectorAccessor
	{
		float *ptr;
		template<typename Vec>
		inline VectorAccessor& operator = (const Vec &rhs)
		{
			*(Vec*)ptr = rhs;
			return *this;
		}
		template<typename Vec>
		inline bool operator == (const Vec &rhs)
		{
			return *(Vec*)ptr == rhs;
		}
		inline float& operator[] (int n)
		{
			return ptr[n];
		}
		inline float operator[] (int n) const
		{
			return ptr[n];
		}
	};

	class CAttributeIterator : public std::iterator<std::random_access_iterator_tag, float>
	{
		typedef CAttributeIterator MyType;
	public:
		CAttributeIterator()
			: m_cursor(nullptr), m_stride(0)
		{
		}
		CAttributeIterator(char *beg, size_t stride = 0)
			: m_cursor(beg), m_stride(0)
		{
		}
		CAttributeIterator(const MyType &other)
		{
			*this = other;
		}
		inline MyType& operator = (const MyType &other)
		{
			m_cursor = other.m_cursor;
			m_stride = other.m_stride;
			return *this;
		}

		inline VectorAccessor operator * ()
		{
			return{ (float*)m_cursor };
		}

		inline MyType& operator ++ ()
		{
			m_cursor += m_stride;
			return *this;
		}
		inline MyType& operator ++ (int)
		{
			MyType _tmp = *this;
			++*this;
			return _tmp;
		}
		inline MyType& operator -- ()
		{
			m_cursor -= m_stride;
			return *this;
		}
		inline MyType& operator -- (int)
		{
			MyType _tmp = *this;
			--*this;
			return _tmp;
		}

		inline MyType& operator += (int n)
		{
			return *this;
		}
		inline MyType& operator -= (int n)
		{
			return *this;
		}
		inline MyType operator + (int n)
		{
			return *this;
		}
		inline MyType operator - (int n)
		{
			return *this;
		}

		inline bool operator == (const MyType& rhs) const
		{
			return m_cursor == rhs.m_cursor;
		}
		inline bool operator != (const MyType& rhs) const
		{
			return m_cursor != rhs.m_cursor;
		}
		inline bool operator < (const MyType& rhs) const
		{
			return m_cursor < rhs.m_cursor;
		}
		inline bool operator > (const MyType& rhs) const
		{
			return m_cursor > rhs.m_cursor;
		}
		inline bool operator <= (const MyType& rhs) const
		{
			return m_cursor <= rhs.m_cursor;
		}
		inline bool operator >= (const MyType& rhs) const
		{
			return m_cursor >= rhs.m_cursor;
		}

	private:
		char *m_cursor;
		unsigned m_stride;
	};

	class CAttributeArray
	{
		typedef CAttributeArray MyType;
	public:
		CAttributeArray()
			: m_beg(nullptr)
			, m_end(nullptr)
			, m_stride(0)
		{}
		CAttributeArray(char *beg, char *end, unsigned stride)
			: m_beg(beg)
			, m_end(end)
			, m_stride(stride)
		{
			assert(m_end - m_beg >= long(m_stride));
		}

		CAttributeArray(const MyType& other)
		{
			*this = other;
		}
		MyType& operator = (const MyType& other)
		{
			m_beg = other.m_beg;
			m_end = other.m_end;
			m_stride = other.m_stride;
		}


		typedef CAttributeIterator iterator;
		inline iterator begin()
		{
			return iterator(m_beg, m_stride);
		}
		inline iterator end()
		{
			return iterator(m_end);
		}
		operator bool() const
		{
			return m_beg < m_end;
		}

		void test_iterator()
		{
			iterator iter(m_beg, m_stride);
			iterator end(m_end);
			Imath::V3f v3;
			float f = 3.14f;
			*iter = v3;
			*iter = f;
			++iter;
			--iter;
			iter += 1;
			iter -= 1;
			iterator i2 = iter++;
			i2 = iter--;
			i2 = iter + 1;
			i2 = iter - 1;
			iter == end;
			iter != end;
			iter < end;
			iter > end;
			iter <= end;
			iter >= end;
			for (auto v : *this)
			{
				v = v3;
				v[0] = 3.14f;
			}
		}

	private:
		char *m_beg;
		char *m_end;
		unsigned m_stride;
	};

	class CVertexBuffer
	{
	public:
		CVertexBuffer();
		CVertexBuffer(const CVertexBuffer&) = delete;
		CVertexBuffer& operator = (const CVertexBuffer&) = delete;
		~CVertexBuffer();

		void resize(unsigned vertex_count);
		void clear();
		
		void set_attribute(EAttributeUsage usage, uint8_t element_count);
		CAttributeArray get_attribute(EAttributeUsage usage);

		inline size_t size() const
		{
			return m_vert_cnt;
		}
		inline size_t vertex_size() const
		{
			return m_vert_size;
		}
	protected:
		char *m_data;
		unsigned m_vert_size;
		unsigned m_vert_cnt;
		VertexAttribute* m_attr_tbl[ATTR_MAX_COUNT];
	};



} // namespace wyc