#ifndef WYC_HEADER_VERTEX_BUFFER
#define WYC_HEADER_VERTEX_BUFFER

#include <cassert>
#include <cstddef>
#include <vector>
#include "mathex/vecmath.h"

namespace wyc
{

	template<typename VERTEX>
	using xvertex_buffer = std::vector<VERTEX>;

	enum VERTEX_ATTRIBUTE {
		VERTEX_POSITION,
		VERTEX_COLOR,
		VERTEX_NORMAL,
	};

	struct VERTEX_P3
	{
		xvec3f_t position;
	};

	struct VERTEX_P3C3
	{
		xvec3f_t position;
		xvec3f_t color;
	};

	struct VERTEX_P3_UV
	{
		xvec3f_t position;
		xvec2f_t uv;
	};


	template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
	void gen_plane(float w, float h, VERTEX_BUFFER &vertices, INDEX_BUFFER &indices);

//
// Template implementations
//

	template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
	void gen_plane(float w, float h, VERTEX_BUFFER &vertices, INDEX_BUFFER &indices)
	{
		vertices.resize(4);
		indices.resize(6);
		float x = w*0.5f;
		float y = h*0.5f;
		vertices[0].position.set(-x, -y, 0);
		vertices[1].position.set(x, -y, 0);
		vertices[2].position.set(x, y, 0);
		vertices[3].position.set(-x, y, 0);
		indices[0] = 0; indices[1] = 1; indices[2] = 3;
		indices[3] = 3; indices[4] = 1; indices[5] = 2;
	}

}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER