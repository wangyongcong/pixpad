#include <cstring>
#include "driver.h"
#include "xbuffer.h"

namespace wyc
{

xbuffer::xbuffer() 
{
	m_data = 0;
	m_info = 0;
	m_pitch = 0;
	m_width = 0;
	m_height = 0;
}

xbuffer::~xbuffer() 
{
	if (m_data && is_owner()) {
		delete[] m_data;
		m_data = 0;
	}
}

bool xbuffer::create(unsigned w, unsigned h, unsigned size_elem, unsigned char alignment) {
	if (m_data) clear();
	size_t pitch = (w * size_elem + alignment - 1) / alignment * alignment;
	size_t required_bytes = pitch*h;
	if (required_bytes<1) 
		return false;
	m_data = new uint8_t[required_bytes];
	if(m_data == 0)
		return false;
	m_info = ((alignment & 0xF) << 16) | (size_elem & 0xFFFF);
	m_pitch = pitch;
	m_width = w;
	m_height = h;
	return true;
}

void xbuffer::clear() 
{
	if (!m_data)
		return;
	if (is_owner()) {
		delete[] m_data;
		m_data = 0;
	}
	m_data = 0;
	m_info = 0;
	m_pitch = 0;
	m_width = 0;
	m_height = 0;
}

bool xbuffer::share(const xbuffer &buffer)
{
	if (!buffer.m_data) 
		return false;
	if (m_data && is_owner()) {
		delete[] m_data;
	}
	m_data = buffer.m_data;
	m_info = buffer.m_info | BI_SHARED;
	m_pitch = buffer.m_pitch;
	m_width = buffer.m_width;
	m_height = buffer.m_height;
	return true;
}

bool xbuffer::share(const xbuffer &buffer, unsigned x, unsigned y, unsigned w, unsigned h)
{
	if (x >= buffer.m_width || y >= buffer.m_height)
		return false;
	if (0 == w || x + w > buffer.m_width) {
		w = buffer.m_width - x;
	}
	if (0 == h || y + h > buffer.m_height) {
		h = buffer.m_height - y;
	}
	m_data = buffer.m_data + y*buffer.m_pitch + x*buffer.size_elem();
	m_info = buffer.m_info | BI_SHARED;
	m_pitch = buffer.m_pitch;
	m_width = w;
	m_height = h;
	return true;
}

void xbuffer::init(const uint8_t *pdata, unsigned size) {
	unsigned cnt;
	if(size>m_pitch)
		size=m_pitch;
	else 
		cnt=m_pitch/size;
	uint8_t *pbuff=m_data;
	for(unsigned y=0; y<m_height; ++y) {
		uint8_t *pline=pbuff;
		pbuff+=m_pitch;
		for(unsigned i=0; i<cnt; ++i) {
			memcpy(pline,pdata,size);
			pline+=size;
		}
	}
}

}; // end of namespace wyc