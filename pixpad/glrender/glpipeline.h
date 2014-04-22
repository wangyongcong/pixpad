#ifndef WYC_HEADER_GL_PIPELINE
#define WYC_HEADER_GL_PIPELINE

#include <unordered_map>
#include "glrender.h"
#include "pipeline\pipeline.h"

namespace wyc
{
	class xgl_pipeline : public xpipeline
	{
	public:
		xgl_pipeline();
		virtual ~xgl_pipeline();
		virtual void set_viewport(unsigned width, unsigned height);
		virtual bool commit(xvertex_buffer *vertices, xindex_buffer *indices);
		virtual bool set_material(const std::string &name);
		virtual void render();

	private:
		GLuint m_vertex_array;
		GLuint m_vertex_buffer;
		GLuint m_index_buffer;
		GLenum m_index_type;
		unsigned m_index_count;
		const xvertex_attrib* m_attribs;
		unsigned m_attrib_count;
		GLuint m_program;

		static const std::unordered_map<std::type_index, GLenum> ms_gltype;

		bool new_hw_buffers();
		void del_hw_buffers();
	};

}; // namespace wyc

#endif // WYC_HEADER_GL_PIPELINE