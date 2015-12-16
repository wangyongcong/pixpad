#pragma once
#include <cstdint>
#include <iterator>
#include <cassert>

#include "OpenEXR/ImathVec.h"
#include "any_stride_iterator.h"

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

	class CVectorReader
	{
	public:
		CVectorReader(void *ptr)
			: m_ptr((float*)ptr)
		{}
		template<typename T>
		inline operator const T& () const
		{
			return *(T*)m_ptr;
		}
		template<typename T>
		inline bool operator == (const T &rhs) const
		{
			return *(T*)ptr == rhs;
		}
		inline float operator[] (int n) const
		{
			return m_ptr[n];
		}
	protected:
		float *m_ptr;
	};

	class CVectorAccessor : public CVectorReader
	{
	public:
		CVectorAccessor(void *ptr)
			: CVectorReader(ptr)
		{}
		template<typename T>
		inline CVectorAccessor& operator = (const T &rhs)
		{
			*(T*)m_ptr = rhs;
			return *this;
		}
		template<typename T>
		inline operator T& ()
		{
			return *(T*)m_ptr;
		}
		inline float& operator[] (int n)
		{
			return m_ptr[n];
		}
	};

	template<> CVectorReader to_ref<CVectorReader>(void *ptr)
	{
		return{ ptr };
	}

	template<> CVectorAccessor to_ref<CVectorAccessor>(void *ptr)
	{
		return{ ptr };
	}

	class CAttributeIterator : public CAnyStrideIterator<float, CVectorAccessor>
	{
		typedef CAttributeIterator MyType;
	public:
		CAttributeIterator()
			: CAnyStrideIterator()
		{
		}
		CAttributeIterator(char *beg, size_t stride = 0)
			: CAnyStrideIterator(beg, stride)
		{
		}
		CAttributeIterator(const MyType &other)
		{
			*this = other;
		}
		inline MyType& operator = (const MyType &other)
		{
			CAnyStrideIterator::operator=(other);
			return *this;
		}
		//inline CVectorAccessor operator * ()
		//{
		//	return { m_cursor };
		//}
	};

	class CAttributeIteratorConst : public CAnyStrideIterator<float, CVectorReader>
	{
		typedef CAttributeIteratorConst MyType;
	public:
		CAttributeIteratorConst()
			: CAnyStrideIterator()
		{
		}
		CAttributeIteratorConst(char *beg, size_t stride = 0)
			: CAnyStrideIterator(beg, stride)
		{
		}
		CAttributeIteratorConst(const MyType &other)
		{
			*this = other;
		}
		inline MyType& operator = (const MyType &other)
		{
			CAnyStrideIterator::operator = (other);
			return *this;
		}
		//inline CVectorReader operator * ()
		//{
		//	return{ m_cursor };
		//}
	};

	class CAttributeArrayBase
	{
		typedef CAttributeArrayBase MyType;
	public:
		CAttributeArrayBase()
			: m_beg(nullptr)
			, m_end(nullptr)
			, m_stride(0)
		{}
		CAttributeArrayBase(char *beg, char *end, unsigned stride)
			: m_beg(beg)
			, m_end(end)
			, m_stride(stride)
		{
			assert(m_end - m_beg >= long(m_stride));
		}
		CAttributeArrayBase(const MyType& other)
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
		typedef CAttributeIteratorConst const_iterator;
	protected:
		char *m_beg;
		char *m_end;
		unsigned m_stride;
	};

	class CAttributeArray : public CAttributeArrayBase
	{
		typedef CAttributeArray MyType;
	public:
		CAttributeArray()
			: CAttributeArrayBase()
		{}
		CAttributeArray(char *beg, char *end, unsigned stride)
			: CAttributeArrayBase(beg, end, stride)
		{}
		CAttributeArray(const MyType& other)
		{
			*this = other;
		}
		MyType& operator = (const MyType& other)
		{
			CAttributeArrayBase::operator=(other);
			return *this;
		}
		inline iterator begin()
		{
			return{ m_beg, m_stride };
		}
		inline iterator end()
		{
			return{ m_end };
		}
		operator bool() const
		{
			return m_beg < m_end;
		}
	};

	class CAttributeArrayConst : public CAttributeArrayBase
	{
		typedef CAttributeArrayConst MyType;
	public:
		CAttributeArrayConst()
			: CAttributeArrayBase()
		{}
		CAttributeArrayConst(char *beg, char *end, unsigned stride)
			: CAttributeArrayBase(beg, end, stride)
		{}
		CAttributeArrayConst(const MyType& other)
		{
			*this = other;
		}
		MyType& operator = (const MyType& other)
		{
			CAttributeArrayBase::operator=(other);
			return *this;
		}
		inline const_iterator begin() const
		{
			return{ m_beg, m_stride };
		}
		inline const_iterator end() const
		{
			return{ m_end };
		}
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
		CAttributeArrayConst get_attribute(EAttributeUsage usage) const;

		typedef CAttributeIterator iterator;
		typedef CAttributeIteratorConst const_iterator;
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