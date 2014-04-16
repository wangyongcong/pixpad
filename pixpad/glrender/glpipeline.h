#ifndef WYC_HEADER_GL_PIPELINE
#define WYC_HEADER_GL_PIPELINE

#include "glrender.h"
#include "pipeline\pipeline.h"

namespace wyc
{
	class xgl_pipeline : public xpipeline
	{
	public:
		xgl_pipeline();
		virtual ~xgl_pipeline();
		virtual bool create_surface(unsigned format, unsigned width, unsigned height);		
		virtual bool commit(xvertex_buffer &vertices, xindex_buffer &indices);
		virtual void render();
			
	private:
		GLuint m_vertex_array;
		GLuint m_vertex_buffer;
		GLuint m_index_buffer;

		bool new_hw_buffers();
		void del_hw_buffers();
	};

}; // namespace wyc

#endif // WYC_HEADER_GL_PIPELINE