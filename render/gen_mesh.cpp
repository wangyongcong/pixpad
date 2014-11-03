#include "gen_mesh.h"
#include "vector.h"

namespace wyc
{

void box(float s, std::vector<vec3f> &vertices, std::vector<unsigned short> &faces)
{
	s *= 0.5f;
	// top [0 3]  bottom [4 7]
	//     [1 2]         [5 6]
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
		5, 4, 6, 5, 6, 7,  // bottom
		1, 5, 6, 1, 6, 2,  // front
		3, 7, 4, 3, 4, 0,  // back
		0, 4, 5, 0, 5, 1,  // left
		2, 6, 7, 2, 7, 3   // right
	};
}

} // namespace wyc