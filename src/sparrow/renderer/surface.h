#ifndef WYC_HEADER_SURFACE
#define WYC_HEADER_SURFACE

#include <cstdint>
#include <cassert>
#include "util.h"

namespace wyc
{
	class CSurface
	{
	public:
		CSurface();
		~CSurface();
		bool storage(unsigned w, unsigned h, unsigned fragment_size, unsigned char alignment = 4);
		void release();
		bool share(const CSurface &buffer);
		bool share(const CSurface &buffer, unsigned x, unsigned y, unsigned w, unsigned h);
		bool empty() const;
		bool is_owner() const;
		unsigned char alignment() const;
		unsigned row_length() const; // fragment count per row
		unsigned row() const;
		unsigned pitch() const;
		unsigned size() const; // size in bytes
		unsigned fragment_size() const;
		bool validate(void *ptr) const;
		// get buffer data
		uint8_t* get_buffer();
		// clear whole buffer
		void clear(const uint8_t *pdata, unsigned size);
		template<class T>
		void clear(const T& val);
		// get/set fragment at (x, y)
		uint8_t* get(unsigned x, unsigned y);
		const uint8_t* get(unsigned x, unsigned y) const;
		template<class T>
		inline T* get(unsigned x, unsigned y);
		template<class T>
		inline void set(unsigned x, unsigned y, const T& val);
		// get/set scan line
		uint8_t* get_line(unsigned idx);
		template<class T>
		void set_line(unsigned ln, const T& val);
		template<class T>
		void set_line(unsigned ln, const T& val, unsigned begx, unsigned endx);
		// move buffer block
		template<class T>
		void move_block(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h);
	protected:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSurface)

		uint8_t *m_data;
		unsigned m_pitch;
		unsigned m_row_len;
		unsigned m_row;
		uint16_t m_frag_size;
		uint8_t m_alignment;
		bool m_is_share;
	};

	inline bool CSurface::empty() const {
		return m_data == 0;
	}

	inline bool CSurface::is_owner() const {
		return !m_is_share;
	}

	inline unsigned char CSurface::alignment() const {
		return m_alignment;
	}

	inline unsigned CSurface::row_length() const {
		return m_row_len;
	}

	inline unsigned CSurface::row() const {
		return m_row;
	}

	inline unsigned CSurface::pitch() const {
		return m_pitch;
	}

	inline unsigned CSurface::size() const {
		return m_pitch*m_row;
	}

	inline unsigned CSurface::fragment_size() const {
		return m_frag_size;
	}

	inline bool CSurface::validate(void *ptr) const {
		return ptr >= m_data && ptr < m_data + size();
	}

	inline uint8_t* CSurface::get_buffer() {
		return m_data;
	}

	inline uint8_t* CSurface::get_line(unsigned idx) {
		return m_data + idx*m_pitch;
	}

	inline uint8_t* CSurface::get(unsigned x, unsigned y) {
		return m_data + y*m_pitch + x*fragment_size();
	}

	inline const uint8_t* CSurface::get(unsigned x, unsigned y) const {
		return m_data + y*m_pitch + x*fragment_size();
	}

	template<class T>
	inline T* CSurface::get(unsigned x, unsigned y) {
		return (T*)(m_data + y*m_pitch + x*fragment_size());
	}

	template<class T>
	inline void CSurface::set(unsigned x, unsigned y, const T& val) {
		assert(x >= 0 && x < m_row_len);
		assert(y >= 0 && y < m_row);
		*(T*)(m_data + y*m_pitch + x*fragment_size()) = val;
	}

	template<class T>
	void CSurface::set_line(unsigned ln, const T& val) {
		T* iter = (T*)(m_data + ln*m_pitch);
		for (unsigned i = 0; i < m_row_len; ++i) {
			iter[i] = val;
		}
	}

	template<class T>
	void CSurface::set_line(unsigned ln, const T& val, unsigned begx, unsigned endx) {
		T* iter = (T*)(m_data + ln*m_pitch);
		for (unsigned i = begx; i < endx; ++i) {
			iter[i] = val;
		}
	}

	template<class T>
	void CSurface::clear(const T& val)
	{
		assert(sizeof(T) == this->fragment_size());
		uint8_t *pline = m_data;
		for (unsigned y = 0; y < m_row; ++y) {
			T* iter = (T*)pline;
			T* end = iter + m_row_len;
			pline += m_pitch;
			while (iter != end)
				*iter++ = val;
		}
	}

	template<class T>
	void CSurface::move_block(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h)
	{
		if (dstx == srcx && dsty == srcy)
			return;
		int xoff, yoff;
		uint8_t *psrc, *pdst;
		T *src_iter, *dst_iter;
		if (dsty <= srcy) {
			yoff = int(pitch());
			psrc = get_line(srcy);
			pdst = get_line(dsty);
		}
		else {
			yoff = -int(pitch());
			psrc = get_line(srcy + h - 1);
			pdst = get_line(dsty + h - 1);
		}
		if (dstx <= srcx) {
			xoff = 1;
			psrc += srcx*sizeof(T);
			pdst += dstx*sizeof(T);
		}
		else {
			xoff = -1;
			psrc += (srcx + w - 1)*sizeof(T);
			pdst += (dstx + w - 1)*sizeof(T);
		}
		while (h > 0) {
			src_iter = (T*)psrc, dst_iter = (T*)pdst;
			for (unsigned i = 0; i < w; ++i) {
				*dst_iter = *src_iter;
				src_iter += xoff;
				dst_iter += xoff;
			}
			h -= 1;
			psrc += yoff;
			pdst += yoff;
		}
	}

} //namespace wyc

#endif // WYC_HEADER_SURFACE