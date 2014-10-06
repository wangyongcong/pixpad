#ifndef WYC_HEADER_SURFACE
#define WYC_HEADER_SURFACE

#include <cstdint>

namespace wyc
{
	class xsurface
	{
	public:
		xsurface();
		xsurface(const xsurface &buffer) = delete;
		xsurface& operator = (const xsurface &buffer) = delete;
		~xsurface();
		bool storage(unsigned w, unsigned h, unsigned fragment_size, unsigned char alignment = 4);
		void release();
		bool share(const xsurface &buffer);
		bool share(const xsurface &buffer, unsigned x, unsigned y, unsigned w, unsigned h);
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
		enum BUFFER_INFO
		{
			BI_ELEMENT_SIZE = 0xFFFF,
			BI_ALIGNMENT = 0xF0000,
			BI_ALIGNMENT_SHIFT = 16,
			BI_SHARED = 0x100000
		};
		uint8_t *m_data;
		unsigned m_info;
		unsigned m_pitch;
		unsigned m_row_len;
		unsigned m_row;
	};

	inline bool xsurface::empty() const {
		return m_data == 0;
	}

	inline bool xsurface::is_owner() const {
		return 0 == (m_info & BI_SHARED);
	}

	inline unsigned char xsurface::alignment() const {
		return (m_info & BI_ALIGNMENT) >> BI_ALIGNMENT_SHIFT;
	}

	inline unsigned xsurface::row_length() const {
		return m_row_len;
	}

	inline unsigned xsurface::row() const {
		return m_row;
	}

	inline unsigned xsurface::pitch() const {
		return m_pitch;
	}

	inline unsigned xsurface::size() const {
		return m_pitch*m_row;
	}

	inline unsigned xsurface::fragment_size() const {
		return m_info & BI_ELEMENT_SIZE;
	}

	inline bool xsurface::validate(void *ptr) const {
		return ptr >= m_data && ptr < m_data + size();
	}

	inline uint8_t* xsurface::get_buffer() {
		return m_data;
	}

	inline uint8_t* xsurface::get_line(unsigned idx) {
		return m_data + idx*m_pitch;
	}

	inline uint8_t* xsurface::get(unsigned x, unsigned y) {
		return m_data + y*m_pitch + x*fragment_size();
	}

	inline const uint8_t* xsurface::get(unsigned x, unsigned y) const {
		return m_data + y*m_pitch + x*fragment_size();
	}

	template<class T>
	inline T* xsurface::get(unsigned x, unsigned y) {
		return ((T*)(m_data + y*m_pitch)) + x;
	}

	template<class T>
	inline void xsurface::set(unsigned x, unsigned y, const T& val) {
		((T*)(m_data + y*m_pitch))[x] = val;
	}

	template<class T>
	void xsurface::set_line(unsigned ln, const T& val) {
		T* iter = (T*)(m_data + ln*m_pitch);
		for (unsigned i = 0; i<m_row_len; ++i) {
			*iter = val;
			iter += 1;
		}
	}

	template<class T>
	void xsurface::set_line(unsigned ln, const T& val, unsigned begx, unsigned endx) {
		T* iter = (T*)(m_data + ln*m_pitch);
		for (unsigned i = begx; i <= endx; ++i) {
			*iter = val;
			iter += 1;
		}
	}

	template<class T>
	void xsurface::clear(const T& val)
	{
		assert(sizeof(T) == this->fragment_size());
		uint8_t *pline = m_data;
		for (unsigned y = 0; y<m_row; ++y) {
			T* iter = (T*)pline;
			pline += m_pitch;
			for (unsigned i = 0; i<m_row_len; ++i) {
				*iter = val;
				iter += 1;
			}
		}
	}

	template<class T>
	void xsurface::move_block(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h)
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
		while (h>0) {
			src_iter = (T*)psrc, dst_iter = (T*)pdst;
			for (unsigned i = 0; i<w; ++i) {
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