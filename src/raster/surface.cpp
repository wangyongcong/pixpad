#include "surface.h"
#include <cstring>

namespace wyc
{
	xsurface::xsurface()
	{
		m_data = 0;
		m_info = 0;
		m_pitch = 0;
		m_row_len = 0;
		m_row = 0;
	}

	xsurface::~xsurface()
	{
		if (m_data && is_owner()) {
			delete[] m_data;
			m_data = 0;
		}
	}

	bool xsurface::storage(unsigned w, unsigned h, unsigned fragment_size, unsigned char alignment) {
		if (m_data) release();
		size_t pitch = (w * fragment_size + alignment - 1) / alignment * alignment;
		size_t required_bytes = pitch*h;
		if (required_bytes<1)
			return false;
		m_data = new uint8_t[required_bytes];
		if (m_data == 0)
			return false;
		m_info = ((alignment & 0xF) << 16) | (fragment_size & 0xFFFF);
		m_pitch = pitch;
		m_row_len = w;
		m_row = h;
		return true;
	}

	void xsurface::release()
	{
		if (!m_data)
			return;
		if (is_owner()) 
			delete[] m_data;
		m_data = 0;
		m_info = 0;
		m_pitch = 0;
		m_row_len = 0;
		m_row = 0;
	}

	bool xsurface::share(const xsurface &buffer)
	{
		if (!buffer.m_data)
			return false;
		if (m_data && is_owner()) {
			delete[] m_data;
		}
		m_data = buffer.m_data;
		m_info = buffer.m_info | BI_SHARED;
		m_pitch = buffer.m_pitch;
		m_row_len = buffer.m_row_len;
		m_row = buffer.m_row;
		return true;
	}

	bool xsurface::share(const xsurface &buffer, unsigned x, unsigned y, unsigned w, unsigned h)
	{
		if (x >= buffer.m_row_len || y >= buffer.m_row)
			return false;
		if (0 == w || x + w > buffer.m_row_len) {
			w = buffer.m_row_len - x;
		}
		if (0 == h || y + h > buffer.m_row) {
			h = buffer.m_row - y;
		}
		m_data = buffer.m_data + y*buffer.m_pitch + x*buffer.fragment_size();
		m_info = buffer.m_info | BI_SHARED;
		m_pitch = buffer.m_pitch;
		m_row_len = w;
		m_row = h;
		return true;
	}

	void xsurface::clear(const uint8_t *pdata, unsigned size) {
		unsigned cnt;
		if (size>m_pitch)
			size = m_pitch;
		else
			cnt = m_pitch / size;
		uint8_t *pbuff = m_data;
		for (unsigned y = 0; y<m_row; ++y) {
			uint8_t *pline = pbuff;
			pbuff += m_pitch;
			for (unsigned i = 0; i<cnt; ++i) {
				memcpy(pline, pdata, size);
				pline += size;
			}
		}
	}

} // namespace wyc