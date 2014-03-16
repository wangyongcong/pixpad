#ifndef WYC_HEADER_SWPIPELINE
#define	WYC_HEADER_SWPIPELINE

#include "framebuffer.h"
#include "pipeline.h"

namespace wyc
{
	// software pipeline 
	class xsw_pipeline : public xpipeline
	{
	public:
		xsw_pipeline();
		virtual ~xsw_pipeline();
		virtual bool create_surface(unsigned format, unsigned width, unsigned height);
		virtual void beg_frame();
		virtual void end_frame();
		virtual void draw(/*vertex buffer*/);

	private:
		xframe_buffer *m_fbo;
	};
}; // end of namespace wyc

#endif // WYC_HEADER_SWPIPELINE