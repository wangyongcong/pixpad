#pragma once
#include "spw_config.h"
#include "util.h"

#define TILE_SIZE 4
#define FRAGMENT_PER_TILE (TILE_SIZE * TILE_SIZE)
#define FRAGMENT_ALIGNMENT 4

namespace wyc
{	
	template<class T>
	class CSpwTileBuffer
	{
	public:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwTileBuffer)
		CSpwTileBuffer();
		~CSpwTileBuffer();
		
		bool storage(unsigned width, unsigned height);
		void release();

		void clear(const T &v);
		T get(unsigned x, unsigned y) const;
		void set(unsigned x, unsigned y, const T &v);
		T* get_tile(unsigned i);
		void set_tile(unsigned i, const T &v);

		inline unsigned size() const {
			return m_size;
		}
		inline unsigned tile_count() const {
			return m_tile_count;
		}
		inline unsigned width() const {
			return m_width;
		}
		inline unsigned height() const {
			return m_height;
		}
		inline unsigned tile_row() const {
			return m_tile_row;
		}
		inline unsigned tile_col() const {
			return m_tile_col;
		}
		inline unsigned tile_size() const {
			return TILE_SIZE;
		}

	private:
		T* _address(unsigned x, unsigned y) const;

		T * m_buf;
		unsigned m_width, m_height, m_size;
		unsigned m_tile_row, m_tile_col, m_tile_count;
		
	}; // class CSpwTileBuffer
	
	template<class T>
	CSpwTileBuffer<T>::CSpwTileBuffer()
		: m_buf(nullptr)
		, m_width(0)
		, m_height(0)
		, m_size(0)
		, m_tile_row(0)
		, m_tile_col(0)
		, m_tile_count(0)
	{
		static_assert((sizeof(T) & (FRAGMENT_ALIGNMENT - 1)) == 0, "Fragment size alignment error");
		constexpr size_t TILE_SIZE_IN_BYTES = sizeof(T) * FRAGMENT_PER_TILE;
		static_assert((TILE_SIZE_IN_BYTES % CACHE_LINE_SIZE == 0) || (CACHE_LINE_SIZE % TILE_SIZE_IN_BYTES == 0), "Tile size do not fit in cache line");
	}
	
	template<class T>
	CSpwTileBuffer<T>::~CSpwTileBuffer()
	{
		if(m_buf)
			delete [] m_buf;
	}
	
	template<class T>
	bool CSpwTileBuffer<T>::storage(unsigned int width, unsigned int height)
	{
		if(m_buf)
			release();
		m_tile_col = (width + TILE_SIZE - 1) / TILE_SIZE;
		m_tile_row = (height + TILE_SIZE - 1) / TILE_SIZE;
		m_tile_count = m_tile_col * m_tile_row;
		m_width = width;
		m_height = height;
		m_size = m_tile_count * FRAGMENT_PER_TILE;
		assert(m_size >= m_width * m_height);
		m_buf = new T[m_size];
		return true;
	}
	
	template<class T>
	void CSpwTileBuffer<T>::release()
	{
		if(!m_buf)
			return;
		delete [] m_buf;
		m_buf = nullptr;
		m_tile_row = m_tile_col = 0;
		m_tile_count = 0;
		m_size = 0;
		m_width = m_height = 0;
	}
	
	template<class T>
	T* CSpwTileBuffer<T>::_address(unsigned x, unsigned y) const
	{
		unsigned c = x / TILE_SIZE;
		unsigned r = y / TILE_SIZE;
		size_t i2 = (r * m_tile_col + c) * FRAGMENT_PER_TILE;
		c = x % TILE_SIZE;
		r = y % TILE_SIZE;
		unsigned i1 = r * TILE_SIZE + c;
		return m_buf + i1 + i2;
	}
	
	template<class T>
	void CSpwTileBuffer<T>::clear(const T &v) {
		for(unsigned i = 0; i < m_size; ++i)
			m_buf[i] = v;
	}

	template<class T>
	inline T CSpwTileBuffer<T>::get(unsigned x, unsigned y) const {
		return *_address(x, y);
	}

	template<class T>
	inline void CSpwTileBuffer<T>::set(unsigned x, unsigned y, const T &v) {
		*_address(x, y) = v;
	}
	
	template<class T>
	T* CSpwTileBuffer<T>::get_tile(unsigned i)
	{
		assert(i < m_tile_count);
		return m_buf + FRAGMENT_PER_TILE * i;
	}
	
	template<class T>
	void CSpwTileBuffer<T>::set_tile(unsigned i, const T &v)
	{
		assert(i < m_tile_count);
		T *dst = m_buf + FRAGMENT_PER_TILE * i;
		for(T *end = dst + FRAGMENT_PER_TILE; dst < end; ++dst)
		{
			*dst = v;
		}
	}
	
} // namespace wyc
