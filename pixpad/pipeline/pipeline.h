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
		// commit triangles to draw
		virtual bool commit (xvertex_buffer &vertices, xindex_buffer &indices) = 0;
		// do render
		virtual void render() = 0;
		// set/get transform matrix
		void set_transform(const xmat4f_t &mat);
		const xmat4f_t& get_transform() const;
	};
}; // end of namespace wyc

#endif // WYC_HEADER_PIPELINE