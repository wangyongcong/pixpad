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

	class CVertexReader
	{
	public:
		CVertexReader(void *ptr)
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

	class CVertexAccessor : public CVertexReader
	{
	public:
		CVertexAccessor(void *ptr)
			: CVertexReader(ptr)
		{}
		template<typename T>
		inline CVertexAccessor& operator = (const T &rhs)
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

	template<> inline CVertexReader to_ref<CVertexReader>(void *ptr)
	{
		return { ptr };
	}

	template<> inline CVertexAccessor to_ref<CVertexAccessor>(void *ptr)
	{
		return { ptr };
	}

	template<bool IsConstant>
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

		operator bool() const
		{
			return m_beg < m_end;
		}

		template<bool T>
		struct Iterator {};
		template<>
		struct Iterator<true> {
			typedef CAnyStrideIterator<float, CVertexReader> type;
		};
		template<>
		struct Iterator<false> {
			typedef CAnyStrideIterator<float, CVertexAccessor> type;
		};
		using iterator = typename Iterator<IsConstant>::type;
		//typedef Iterator<false>::type iterator;
		//typedef Iterator<true>::type const_iterator;
	
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

	//class CAttributeArray : public CAttributeArrayBase
	//{
	//	typedef CAttributeArray MyType;
	//public:
	//	CAttributeArray()
	//		: CAttributeArrayBase()
	//	{}
	//	CAttributeArray(char *beg, char *end, unsigned stride)
	//		: CAttributeArrayBase(beg, end, stride)
	//	{}
	//	CAttributeArray(const MyType& other)
	//	{
	//		*this = other;
	//	}
	//	MyType& operator = (const MyType& other)
	//	{
	//		CAttributeArrayBase::operator=(other);
	//		return *this;
	//	}
	//	inline iterator begin()
	//	{
	//		return{ m_beg, m_stride };
	//	}
	//	inline iterator end()
	//	{
	//		return{ m_end };
	//	}
	//	operator bool() const
	//	{
	//		return m_beg < m_end;
	//	}
	//};

	//class CAttributeArrayConst : public CAttributeArrayBase
	//{
	//	typedef CAttributeArrayConst MyType;
	//public:
	//	CAttributeArrayConst()
	//		: CAttributeArrayBase()
	//	{}
	//	CAttributeArrayConst(char *beg, char *end, unsigned stride)
	//		: CAttributeArrayBase(beg, end, stride)
	//	{}
	//	CAttributeArrayConst(const MyType& other)
	//	{
	//		*this = other;
	//	}
	//	MyType& operator = (const MyType& other)
	//	{
	//		CAttributeArrayBase::operator=(other);
	//		return *this;
	//	}
	//	inline const_iterator begin() const
	//	{
	//		return{ m_beg, m_stride };
	//	}
	//	inline const_iterator end() const
	//	{
	//		return{ m_end };
	//	}
	//};

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
		CAttributeArray<false> get_attribute(EAttributeUsage usage);
		CAttributeArray<true> get_attribute(EAttributeUsage usage) const;

		typedef CAnyStrideIterator<float, CVertexAccessor> iterator;
		typedef CAnyStrideIterator<float, CVertexReader> const_iterator;
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