#ifndef WYC_HEADER_FRAMEBUFFER
#define WYC_HEADER_FRAMEBUFFER

#include "render_buffer.h"

namespace wyc
{
	class xframe_buffer
	{
	public:
		enum BUFFER_TYPE
		{
			COLOR_BUFFER = 0,
			DEPTH_BUFFER = 1,
			STENCIL_BUFFER = 2,
		};
		virtual ~xframe_buffer() {};
		virtual bool create(unsigned image_format, unsigned width, unsigned height, unsigned depth_format=0, unsigned stencil_format=0) = 0;
		virtual void beg_render() = 0;
		virtual void end_render() = 0;
		virtual xrender_buffer* get_buffer(BUFFER_TYPE t) = 0;
	}; // class xframe_buffer

}; // end of namespace wyc

#endif WYC_HEADER_FRAMEBUFFER