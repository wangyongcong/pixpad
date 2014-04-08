#ifndef WYC_HEADER_GL_VERTEX
#define WYC_HEADER_GL_VERTEX

#include <tuple>
#include "glrender.h"
#include "vertex_buffer.h"

namespace wyc {

	class xgl_vertbuff 
	{
	public:
		xgl_vertbuff();
		~xgl_vertbuff();
		template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
		bool commit(const VERTEX_BUFFER &vertices, const INDEX_BUFFER &indices);
		template<typename VERTEX>
		bool commit(VERTEX *vertices, size_t vertex_count);

	private:
		GLuint m_vertex_array;
		GLuint m_vertex_buffer;
		GLuint m_index_buffer;

		bool new_hw_buffers();
		void del_hw_buffers();

		template<typename VERTEX>
		static void bind_attribs (const std::vector<VERTEX> &buffer);
		template<> static void bind_attribs<vertex_p3>(const std::vector<vertex_p3> &buffer);
		template<> static void bind_attribs<vertex_p3c3>(const std::vector<vertex_p3c3> &buffer);

	};

	template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
	bool commit(const VERTEX_BUFFER &vertices, const INDEX_BUFFER &indices)
	{
		typedef VERTEX_BUFFER::value_t vertex_t;
		if (!m_glvao)
		{
			if (!new_hw_buffers())
				return false;
		}
		
		glBindVertexArray(m_glvao);
		glBindBuffer(GL_ARRAY_BUFFER, m_glvbo);
		size_t size_in_bytes = sizeof(vertex_t)*buffer.size();
		glBufferData(GL_ARRAY_BUFFER, size_in_bytes, &buffer[0], GL_STATIC_DRAW);
		// Biding begin
		bind_attribs<vertex_t>(buffer);
		// Biding end
		glBindVertexArray(0);
		return true;
	}

	template<typename VERTEX>
	bool commit(VERTEX *vertices, size_t vertex_count)
	{
		typedef VERTEX vertex_t;
		if (!m_glvao)
		{
			if (!new_hw_buffers())
				return false;
		}

		glBindVertexArray(m_glvao);
		glBindBuffer(GL_ARRAY_BUFFER, m_glvbo);
		size_t size_in_bytes = sizeof(vertex_t)*vertex_count;
		glBufferData(GL_ARRAY_BUFFER, size_in_bytes, vertices, GL_STATIC_DRAW);
		// Biding begin
		bind_attribs<vertex_t>(buffer);
		// Biding end
		glBindVertexArray(0);
		return true;
	}

	template<> void xgl_vertbuff::bind_attribs<vertex_p3>(const std::vector<vertex_p3> &buffer)
	{
		typedef vertex_p3 vertex_t;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_t), &buffer[0].position);
		glEnableVertexAttribArray(0);
	}

	template<> void xgl_vertbuff::bind_attribs<vertex_p3c3>(const std::vector<vertex_p3c3> &buffer)
	{
		typedef vertex_p3c3 vertex_t;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_t), &buffer[0].position);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_t), &buffer[0].color);
		glEnableVertexAttribArray(1);
	}


}; // end of namespace wyc

#endif // WYC_HEADER_GL_VERTEX