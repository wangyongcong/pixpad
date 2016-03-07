#pragma once

#include <iterator>
#include <type_traits>

namespace wyc
{
	class CAnyReader
	{
	public:
		CAnyReader(void *ptr)
			: m_ptr(ptr)
		{}
		template<typename T>
		inline operator const T& () const
		{
			return *reinterpret_cast<const T*>(m_ptr);
		}
		template<typename T>
		inline operator T& () const
		{
			static_assert(0, "Can't convert to non-const reference.");
		}
		template<typename T>
		inline bool operator == (const T &rhs) const
		{
			return (void*)m_ptr == (void*)rhs;
		}
	private:
		void *m_ptr;
	};

	class CAnyAccessor
	{
	public:
		CAnyAccessor(const CAnyAccessor&) = delete;
		CAnyAccessor& operator = (const CAnyAccessor&) = delete;
		template<typename T>
		inline CAnyAccessor& operator = (const T &rhs)
		{
			*reinterpret_cast<T*>(this) = rhs;
			return *this;
		}
		template<typename T>
		inline operator T& ()
		{
			return *reinterpret_cast<T*>(this);
		}
	};

	template<typename T>
	inline T to_ref(void *ptr)
	{
		return *reinterpret_cast<std::remove_reference<T>::type*>(ptr);
	}

	template<> inline CAnyReader to_ref<CAnyReader>(void *ptr)
	{
		return{ ptr };
	}

	template<typename T, typename Ref=T&, typename Ptr=T*>
	class CAnyStrideIterator : public std::iterator<std::random_access_iterator_tag, T>
	{
		typedef CAnyStrideIterator MyType;
	public:
		CAnyStrideIterator()
			: m_cursor(nullptr), m_stride(0)
		{
		}
		CAnyStrideIterator(void *beg, size_t stride = 0)
			: m_cursor((char*)beg), m_stride(stride)
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

		inline Ref operator * ()
		{
			return to_ref<Ref>(m_cursor);
		}
		inline Ptr operator -> ()
		{
			return reinterpret_cast<Ptr>(m_cursor);
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