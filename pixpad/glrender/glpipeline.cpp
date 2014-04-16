#include "glpipeline.h"

namespace wyc
{
	xgl_pipeline::xgl_pipeline()
	{
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;

	}

	xgl_pipeline::~xgl_pipeline()
	{
		if (m_vertex_array)
			del_hw_buffers();
	}

	void xgl_pipeline::del_hw_buffers()
	{
		assert(m_vertex_array && m_vertex_buffer && m_index_buffer);
		glDeleteVertexArrays(1, &m_vertex_array);
		GLuint vbo[2] = { m_vertex_buffer, m_index_buffer };
		glDeleteBuffers(2, vbo);
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;
	}

	bool xgl_pipeline::new_hw_buffers()
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

	bool xgl_pipeline::create_surface(unsigned format, unsigned width, unsigned height)
	{
		return true;
	}

	bool xgl_pipeline::commit(xvertex_buffer &vertices, xindex_buffer &indices)
	{
		if (!m_vertex_array) {
			if (!new_hw_buffers())
				return false;
		}
		glBindVertexArray(m_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size_in_bytes(), vertices.get_data(), GL_STATIC_DRAW);
		// bind vetex attributes
		unsigned count = vertices.attrib_count();
		const xvertex_attrib* attribs = vertices.get_attribs();
		for (unsigned i = 0; i < count; ++i)
		{
			
		}
		glBindVertexArray(0);
		return true;
	}


}; // namespace wyc
