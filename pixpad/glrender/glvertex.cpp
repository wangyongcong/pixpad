#include "glvertex.h"

namespace wyc
{
	xgl_vertbuff::xgl_vertbuff()
	{
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;
	}

	xgl_vertbuff::~xgl_vertbuff()
	{
		if (m_vertex_array)
			del_hw_buffers();
	}

	bool xgl_vertbuff::new_hw_buffers()
	{
		assert(!m_vertex_array && !m_vertex_buffer && !m_index_buffer);
		GLuint vao;
		glGenVertexArrays(1, &vao);
		if (!vao)
			return false;
		GLuint vbo[2];
		glGenBuffers(2, vbo);
		if (!vbo[0] || !vbo[1]) {
			glDeleteVertexArrays(1, &vao);
			return false;
		}
		m_vertex_array = vao;
		m_vertex_buffer = vbo[0];
		m_index_buffer = vbo[1];
		return true;
	}

	void xgl_vertbuff::del_hw_buffers()
	{
		assert(m_vertex_array && m_vertex_buffer && m_index_buffer);
		glDeleteVertexArrays(1, &m_vertex_array);
		GLuint vbo[2] = { m_vertex_buffer, m_index_buffer };
		glDeleteBuffers(2, vbo);
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;
	}


}; // end of namespace
