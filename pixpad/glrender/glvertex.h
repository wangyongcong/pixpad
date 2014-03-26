#ifndef WYC_HEADER_GL_VERTEX
#define WYC_HEADER_GL_VERTEX

#include "glrender.h"
#include "vertex_buffer.h"

namespace wyc {

	class xgl_vertbuff 
	{
	public:
		xgl_vertbuff();
		~xgl_vertbuff();
		template<typename VERTEX>
		bool commit(xvertex_buffer<VERTEX> &buffer);
		template<>
		bool commit<VERTEX_P3C3>(xvertex_buffer<VERTEX_P3C3> &buffer)
		{
			if (!m_glvao)
			{
				// TODO: create vertex array object
				return false;
			}
			if (!m_glvbo)
			{
				// TODO: create vertex buffer object
				return false;
			}
			glBindVertexArray(m_glvao);
			glBindBuffer(GL_ARRAY_BUFFER,m_glvbo);
			glBufferData(GL_ARRAY_BUFFER, buffer.size_in_bytes(), buffer.get_data(), GL_STATIC_DRAW);
			// Biding begin
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(VERTEX_P3C3), buffer.get_ptr<VERTEX_POSITION>());
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(VERTEX_P3C3), buffer.get_ptr<VERTEX_COLOR>());
			glEnableVertexAttribArray(1);
			// Biding end
			glBindVertexArray(0);
			return true;
		}
	private:
		GLuint m_glvao;
		GLuint m_glvbo;
		GLenum m_prim_type;
	};



}; // end of namespace wyc

#endif // WYC_HEADER_GL_VERTEX