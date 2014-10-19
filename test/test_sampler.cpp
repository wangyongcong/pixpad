#include <cassert>
#include "raster.h"
#include "vecmath.h"
#include "test_sampler.h"

namespace test
{
	void test_polygon_clipping()
	{
		printf("Testint polygon clipping...");
		
		std::vector<vec4f_t> planes;
		// left plane
		planes.push_back(vec4f_t(1, 0, 0, 1));
		// top plane
		planes.push_back(vec4f_t(0, -1, 0, -1));
		// right plane
		planes.push_back(vec4f_t(-1, 0, 0, -1));
		// bottom plane
		planes.push_back(vec4f_t(0, 1, 0, 1));

		std::vector<vec3f_t> vertices;
		vertices.push_back(vec3f_t(0, 0, 0));
		vertices.push_back(vec3f_t(-1, -2, 0));
		vertices.push_back(vec3f_t(1, -2, 0));
		wyc::clip_polygon(planes, vertices);
		for (auto &vec : vertices)
		{
			assert(vec.x >= -1 && vec.x <= 1);
			assert(vec.y >= -1 && vec.y <= 1);
			assert(vec.z >= -1 && vec.z <= 1);
		}

		printf("Ok\n");
	}

} // namespace test