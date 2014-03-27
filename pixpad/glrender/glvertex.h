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
		template<typename VERTEX>
		bool commit(const xvertex_buffer<VERTEX> &buffer);

	private:
		GLuint m_glvao;
		GLuint m_glvbo;

		bool new_hw_buffers();
		void del_hw_buffers();

		template<typename VERTEX>
		void bind_attribs (const xvertex_buffer<VERTEX> &buffer);
		template<> inline void bind_attribs<VERTEX_P3>(const xvertex_buffer<VERTEX_P3> &buffer);
		template<> inline void bind_attribs<VERTEX_P3C3>(const xvertex_buffer<VERTEX_P3C3> &buffer);

	};

	template<typename VERTEX>
	bool xgl_vertbuff::commit(const xvertex_buffer<VERTEX> &buffer)
	{
		typedef VERTEX vertex_t;
		if (!m_glvao)
		{
			if (!new_hw_buffers())
				return false;
		}
		if (!m_glvbo)
		{
			// TODO: create vertex buffer object
			return false;
		}
		glBindVertexArray(m_glvao);
		glBindBuffer(GL_ARRAY_BUFFER, m_glvbo);
		glBufferData(GL_ARRAY_BUFFER, buffer.size_in_bytes(), buffer.get_data(), GL_STATIC_DRAW);
		// Biding begin
		bind_attribs<VERTEX>(buffer);
		// Biding end
		glBindVertexArray(0);
		return true;
	}

	template<> inline void xgl_vertbuff::bind_attribs<VERTEX_P3>(const xvertex_buffer<VERTEX_P3> &buffer)
	{
		typedef VERTEX_P3 vertex_t;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_t), buffer.get_ptr<VERTEX_POSITION>());
		glEnableVertexAttribArray(0);
	}

	template<> inline void xgl_vertbuff::bind_attribs<VERTEX_P3C3>(const xvertex_buffer<VERTEX_P3C3> &buffer)
	{
		typedef VERTEX_P3C3 vertex_t;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(VERTEX_P3C3), buffer.get_ptr<VERTEX_POSITION>());
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_t), buffer.get_ptr<VERTEX_COLOR>());
		glEnableVertexAttribArray(1);
	}


}; // end of namespace wyc

#endif // WYC_HEADER_GL_VERTEX