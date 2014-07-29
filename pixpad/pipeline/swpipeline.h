#ifndef WYC_HEADER_SWPIPELINE
#define	WYC_HEADER_SWPIPELINE

#include "framebuffer.h"
#include "pipeline.h"
#include "xraster.h"
#include "glrender.h"

namespace wyc
{
	// software pipeline 
	class xsw_pipeline : public xpipeline
	{
	public:
		xsw_pipeline();
		virtual ~xsw_pipeline();
		virtual void set_viewport(unsigned width, unsigned height);
		virtual bool set_material(const std::string &name);
		virtual void draw(xvertex_buffer *vertices, xindex_buffer *indices);
		virtual void flush();

	private:
		xframe_buffer *m_fbo;
		xvertex_buffer *m_vertices;
		xindex_buffer *m_indices;
		GLuint m_texobj;
		xraster m_raster;
	};
}; // end of namespace wyc

#endif // WYC_HEADER_SWPIPELINE