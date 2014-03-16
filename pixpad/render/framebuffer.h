#ifndef WYC_HEADER_FRAMEBUFFER
#define WYC_HEADER_FRAMEBUFFER

namespace wyc
{
	class xframe_buffer
	{
	public:
		virtual ~xframe_buffer() = 0;
		virtual bool create(unsigned image_format, unsigned width, unsigned height, unsigned depth_format=0, unsigned stencil_format=0) = 0;
		virtual void beg_render() = 0;
		virtual void end_render() = 0;
	}; // class xframe_buffer

}; // end of namespace wyc

#endif WYC_HEADER_FRAMEBUFFER