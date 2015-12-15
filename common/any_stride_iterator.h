#pragma once

#include <iterator>

namespace wyc
{

	template<typename T>
	class CAnyStrideIterator : public std::iterator<std::random_access_iterator_tag, T>
	{
		typedef CAnyStrideIterator MyType;
	public:
		CAnyStrideIterator()
			: m_cursor(nullptr), m_stride(0)
		{
		}
		CAnyStrideIterator(char *beg, size_t stride = 0)
			: m_cursor(beg), m_stride(stride)
		{
		}
		CAnyStrideIterator(const MyType &other)
		{
			*this = other;
		}
		inline MyType& operator = (const MyType &other)
		{
			m_cursor = other.m_cursor;
			m_stride = other.m_stride;
			return *this;
		}

		inline T& operator * ()
		{
			return *(T*)m_cursor;
		}
		inline T* operator -> ()
		{
			return (T*)m_cursor;
		}

		inline MyType& operator ++ ()
		{
			m_cursor += m_stride;
			return *this;
		}
		inline MyType operator ++ (int)
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
		inline MyType operator -- (int)
		{
			MyType _tmp = *this;
			--*this;
			return _tmp;
		}

		inline MyType& operator += (int n)
		{
			m_cursor += m_stride * n;
			return *this;
		}
		inline MyType& operator -= (int n)
		{
			m_cursor -= m_stride * n;
			return *this;
		}
		inline MyType operator + (int n) const
		{
			return{ m_cursor + m_stride * n, m_stride };
		}
		inline MyType operator - (int n) const
		{
			return{ m_cursor - m_stride * n, m_stride };
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

	protected:
		char *m_cursor;
		unsigned m_stride;
	};

} // namespace wyc