#ifndef WYC_HEADER_GEN_MESH
#define WYC_HEADER_GEN_MESH

#include <vector>
#include "mathfwd.h"

namespace wyc
{
	void box(float size, std::vector<vec3f> &vertices, std::vector<unsigned short> &faces);
}

#endif // WYC_HEADER_GEN_MESH