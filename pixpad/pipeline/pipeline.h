#ifndef WYC_HEADER_PIPELINE
#define WYC_HEADER_PIPELINE

namespace wyc
{
	class xpipeline
	{
	public:
		virtual ~xpipeline() {};
		virtual bool create_surface(unsigned format, unsigned width, unsigned height) = 0;
		virtual void beg_frame() = 0;
		virtual void end_frame() = 0;
		virtual void draw(/*vertex buffer*/) = 0;
	};
}; // end of namespace wyc

#endif // WYC_HEADER_PIPELINE