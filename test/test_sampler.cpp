#include <cassert>

#include <OpenEXR/ImathRandom.h>

#include "raster.h"
#include "vecmath.h"
#include "test_sampler.h"

namespace test
{
	void check_clippint_result(const std::vector<vec3f_t> &vertices)
	{
		float epsilon = std::numeric_limits<float>::epsilon();
		epsilon *= 2;
		for (const auto &v : vertices)
		{
			assert(v.x + 1 >= -epsilon);
			assert(v.x - 1 <=  epsilon);
			assert(v.y + 1 >= -epsilon);
			assert(v.y - 1 <=  epsilon);
			//assert(v.x >= -1 && v.x <= 1);
			//assert(v.y >= -1 && v.y <= 1);
		}
	}

	void test_polygon_clipping()
	{
		printf("Testint polygon clipping...");
		
		std::vector<vec4f_t> planes;
		// left plane
		planes.push_back(vec4f_t(1, 0, 0, 1));
		// top plane
		planes.push_back(vec4f_t(0, -1, 0, 1));
		// right plane
		planes.push_back(vec4f_t(-1, 0, 0, 1));
		// bottom plane
		planes.push_back(vec4f_t(0, 1, 0, 1));

		std::vector<vec3f_t> vertices;

		//-------------------------------------------------------------
		// Fail case
		std::vector<vec3f_t> fail_cases = {
			{ 0.755790f, -1.587896f, 0 },
			{ -0.906151f, 0.777092f, 0 },
			{ -0.298522f, -1.857425f, 0 },
		};
		for (int i = 2, c = fail_cases.size(); i < c; i += 3)
		{
			vertices.clear();
			vertices.push_back(fail_cases[i-2]);
			vertices.push_back(fail_cases[i-1]);
			vertices.push_back(fail_cases[i]);
			wyc::clip_polygon(planes, vertices);
			check_clippint_result(vertices);
		}
		//-------------------------------------------------------------

		IMATH_NAMESPACE::Rand32 rnd(clock());
		for (int c = 0; c < 1000; ++c)
		{
			vertices.clear();
			for (int i = 0; i < 3; ++i)
			{
				float x = (float)rnd.nextf();
				float y = (float)rnd.nextf();
				// clamp to [-2, 2]
				x = (x - 0.5f) * 4;
				y = (y - 0.5f) * 4;
				vertices.push_back(vec3f_t(x, y, 0));
				printf("v%d=(%f, %f) ", i, x, y);
			}
			printf("\n");
			wyc::clip_polygon(planes, vertices);
			check_clippint_result(vertices);
		}

		printf("Test Ok!\n");
	}

} // namespace test