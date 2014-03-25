#include "vertex_buffer.h"

namespace wyc
{

	void generate_cube (xvertex_buffer<VERTEX_P3C3>& buffer, float size)
	{
		size *= 0.5f;
		float vertex[24] = {
			// front face
			-size, size, size,
			-size, -size, size,
			size, -size, size,
			size, size, size,
			// back face
			size, size, -size,
			size, -size, -size,
			-size, -size, -size,
			-size, size, -size,
		};
		unsigned short index[36] = {

		};
		buffer.storage(8);
		for (size_t i = 0, j = 0; i < buffer.size(); ++i, j+=3)
		{
			buffer[i].position.set(vertex[j], vertex[j + 1], vertex[j + 2]);
		}
	}

}