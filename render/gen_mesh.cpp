#include "gen_mesh.h"

using namespace wyc;

void box(float s, std::vector<vec3f_t> &vertices, std::vector<unsigned short> &faces)
{
	s *= 0.5f;
	// top [0 3]  bottom [5 8]
	//     [1 2]         [6 7]
	vertices = {
		{ -s, s, -s },
		{ -s, s, s },
		{ s, s, s },
		{ s, s, -s },
		{ -s, -s, -s },
		{ -s, -s, s },
		{ s, -s, s },
		{ s, -s, -s },
	};
	faces = {
		0, 1, 2, 0, 2, 3,  // top
		6, 5, 7, 6, 7, 8,  // bottom
		1, 6, 7, 1, 7, 2,  // front
		3, 8, 5, 3, 5, 0,  // back
		0, 5, 6, 0, 6, 1,  // left
		2, 7, 8, 2, 8, 3   // right
	};
}
