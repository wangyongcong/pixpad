#include <cassert>

#include <OpenEXR/ImathRandom.h>

#include "raster.h"
#include "vecmath.h"
#include "test_sampler.h"

namespace test
{
	void check_clipping_result(const std::vector<vec3f_t> &vertices)
	{
		float epsilon = std::numeric_limits<float>::epsilon();
		epsilon *= 2;
		for (const auto &v : vertices)
		{
			assert(v.x + 1 >= -epsilon);
			assert(v.x - 1 <=  epsilon);
			assert(v.y + 1 >= -epsilon);
			assert(v.y - 1 <=  epsilon);
		}
	}

	void check_clipping_result(const std::vector<vec4f_t> &vertices, float l, float r, float b, float t, float n, float f)
	{
		float epsilon = std::numeric_limits<float>::epsilon();
		epsilon *= 2;
		for (const auto &v : vertices)
		{
			assert(v.w > 0);
			assert(v.x / v.w - l >= -epsilon);
			assert(v.x / v.w - r <=  epsilon);
			assert(v.y / v.w - b >= -epsilon);
			assert(v.y / v.w - t <=  epsilon);
			//assert(v.z - n >= -epsilon);
			//assert(v.z - f <= epsilon);
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
			check_clipping_result(vertices);
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
			check_clipping_result(vertices);
		}

		printf("Test Ok!\n");
	}

	void test_homogeneous_clipping()
	{
		printf("Testint polygon clipping in homogeneous space...\n");

		std::vector<vec4f_t> vertices;
		float fov = 45, aspect = 4.0f / 3;
		float l, r, t, b, n = 1, f = 100;
		t = n*tan(wyc::deg2rad(fov*0.5f));
		b = -t;
		r = t*aspect;
		l = -r;
		mat4f_t proj;
		vec4f_t v;
		wyc::set_perspective(proj, fov, aspect, n, f);

		//-------------------------------------------------------------
		// Fail case
		std::vector<vec4f_t> fail_cases = {
				{ 20.409956f, -16.483150f, 31.582451f, 32.937252f },
				{ 11.636814f, 34.544147f, -3.122203f, -1.080179f },
				{ -50.721680f, -95.224144f, -4.018780f, -1.959002f },
		};
		for (int i = 2, c = fail_cases.size(); i < c; i += 3)
		{
			vertices.clear();
			vertices.push_back(fail_cases[i - 2]);
			vertices.push_back(fail_cases[i - 1]);
			vertices.push_back(fail_cases[i]);
			wyc::clip_polygon(vertices);
			check_clipping_result(vertices, l, r, b, t, n, f);
		}
		//-------------------------------------------------------------

		IMATH_NAMESPACE::Rand32 rnd(clock());
		for (int c = 0; c < 10; ++c)
		{
			vertices.clear();
			for (int i = 0; i < 3; ++i)
			{
				float x = (float)rnd.nextf();
				float y = (float)rnd.nextf();
				float z = (float)rnd.nextf();
				// clamp to [-50, 50]
				v.x = (x - 0.5f) * 100;
				v.y = (y - 0.5f) * 100;
				v.z = (z - 0.5f) * 100;
				v.w = 1;
				v = proj * v;
				vertices.push_back(v);
				printf("v%d=(%ff, %ff, %ff, %ff) ", i, v.x, v.y, v.z, v.w);
			}
			printf("\n");
			wyc::clip_polygon(vertices);
			check_clipping_result(vertices, l, r, b, t, n, f);
		}

	}

} // namespace test