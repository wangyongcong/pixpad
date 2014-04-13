#include "vertex_buffer.h"

namespace wyc
{
	void test()
	{
		xvertex_buffer vertices;
		xindex_buffer indices;
		gen_regular_triangle<vertex_p3c3>(1, vertices, indices);

	}
}