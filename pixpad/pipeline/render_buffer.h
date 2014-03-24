#ifndef WYC_HEADER_XBUFFER
#define WYC_HEADER_XBUFFER

#include <cstdint>
#include "driver.h"

namespace wyc
{

//==xbuffer===============================================================================================

class xbuffer
{
public:
	xbuffer();
	xbuffer(const xbuffer &buffer) = delete;
	xbuffer& operator = (const xbuffer &buffer) = delete;
	~xbuffer();
	bool storage(unsigned w, unsigned h, unsigned size_elem, unsigned char alignment=4);
	void release();
	bool share(const xbuffer &buffer);
	bool share(const xbuffer &buffer, unsigned x, unsigned y, unsigned w, unsigned h);

	bool empty() const;
	bool is_owner() const;
	unsigned char alignment() const;
	unsigned width() const;
	unsigned height() const;
	unsigned pitch() const;
	unsigned size() const;
	unsigned size_elem() const;

	uint8_t* get_buffer();
	uint8_t* get_line(unsigned idx);
	uint8_t* get_elem(unsigned x, unsigned y);
	const uint8_t* get_elem(unsigned x, unsigned y) const;

	template<class T>
	inline T& get(unsigned x, unsigned y);
	template<class T>
	inline void set(unsigned x, unsigned y, const T& val);

	void clear(const uint8_t *pdata, unsigned size);
	template<class T>
	void clear(const T& val);

	template<class T>
	void set_line(unsigned ln, const T& val);
	template<class T>
	void set_line(unsigned ln, const T& val, unsigned begx, unsigned endx);

	template<class T> 
	void move_elem(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h);
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
	unsigned m_width;
	unsigned m_height;
};

inline bool xbuffer::empty() const {
	return m_data == 0;
}

inline bool xbuffer::is_owner() const {
	return 0 == (m_info & BI_SHARED);
}

inline unsigned char xbuffer::alignment() const {
	return (m_info & BI_ALIGNMENT) >> BI_ALIGNMENT_SHIFT;
}

inline unsigned xbuffer::width() const {
	return m_width;
}

inline unsigned xbuffer::height() const {
	return m_height;
}

inline unsigned xbuffer::pitch() const {
	return m_pitch;
}

inline unsigned xbuffer::size() const {
	return m_pitch*m_height;
}

inline unsigned xbuffer::size_elem() const {
	return m_info & BI_ELEMENT_SIZE;
}

inline uint8_t* xbuffer::get_buffer() {
	return m_data;
}

inline uint8_t* xbuffer::get_line(unsigned idx) {
	return m_data + idx*m_pitch;
}

inline uint8_t* xbuffer::get_elem(unsigned x, unsigned y) {
	return m_data + y*m_pitch + x*size_elem();
}

inline const uint8_t* xbuffer::get_elem(unsigned x, unsigned y) const {
	return m_data + y*m_pitch + x*size_elem();
}

template<class T>
inline T& xbuffer::get(unsigned x, unsigned y) {
	return ((T*)(m_data + y*m_pitch))[x];
}

template<class T>
inline void xbuffer::set(unsigned x, unsigned y, const T& val) {
	((T*)(m_data + y*m_pitch))[x] = val;
}

template<class T>
void xbuffer::set_line(unsigned ln, const T& val) {
	T* iter = (T*)(m_data + ln*m_pitch);
	for (unsigned i = 0; i<m_width; ++i) {
		*iter = val;
		iter += 1;
	}
}

template<class T>
void xbuffer::set_line(unsigned ln, const T& val, unsigned begx, unsigned endx) {
	T* iter = (T*)(m_data + ln*m_pitch);
	for (unsigned i = begx; i <= endx; ++i) {
		*iter = val;
		iter += 1;
	}
}

template<class T>
void xbuffer::clear(const T& val)
{
	uint8_t *pline = m_data;
	for (unsigned y = 0; y<m_height; ++y) {
		T* iter = (T*)pline;
		pline += m_pitch;
		for (unsigned i = 0; i<m_width; ++i) {
			*iter = val;
			iter += 1;
		}
	}
}

template<class T>
void xbuffer::move_elem(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h)
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

}; // end of namespace wyc

#endif // end of WYC_HEADER_XBUFFER
