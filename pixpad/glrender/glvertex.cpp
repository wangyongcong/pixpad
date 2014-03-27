#include "glvertex.h"

namespace wyc
{
	xgl_vertbuff::xgl_vertbuff()
	{
		m_glvao = 0;
		m_glvbo = 0;
	}

	xgl_vertbuff::~xgl_vertbuff()
	{
		if (m_glvao)
			del_hw_buffers();
	}

	bool xgl_vertbuff::new_hw_buffers()
	{
		assert(!m_glvao && !m_glvbo);
		glGenVertexArrays(1, &m_glvao);
		if (!m_glvao)
			return false;
		glGenBuffers(1, &m_glvbo);
		if (!m_glvbo) {
			glDeleteVertexArrays(1, &m_glvao);
			m_glvao = 0;
			return false;
		}
		return true;
	}

	void xgl_vertbuff::del_hw_buffers()
	{
		assert(m_glvao && m_glvbo);
		glDeleteVertexArrays(1, &m_glvao);
		glDeleteBuffers(1, &m_glvbo);
		m_glvao = 0;
		m_glvbo = 0;
	}


}; // end of namespace
