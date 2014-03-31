#ifndef WYC_HEADER_PIPELINE
#define WYC_HEADER_PIPELINE

#include "vertex_buffer.h"

namespace wyc
{
	class xpipeline
	{
	public:
		virtual ~xpipeline() {};
		virtual bool create_surface(unsigned format, unsigned width, unsigned height) = 0;
		virtual void beg_frame() = 0;
		virtual void end_frame() = 0;
		virtual void draw(xvertex_buffer &vertices, xindex_buffer &indices) = 0;
	};
}; // end of namespace wyc

#endif // WYC_HEADER_PIPELINE