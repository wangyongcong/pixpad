#include <cassert>
#include <ctime>
#include <initializer_list>
#include <OpenEXR/ImathRandom.h>

#include "raster.h"
#include "vecmath.h"
#include "vector.h"
#include "matrix.h"
#include "test_sampler.h"

using namespace wyc;

namespace test
{

	template<typename VECTOR>
	void check_clipping(const std::vector<VECTOR> &vertices)
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

	template<typename VECTOR>
	void check_clipping_strict(const std::vector<VECTOR> &vertices)
	{
		for (const auto &v : vertices)
		{
			assert(v.x > -1 && v.x < 1);
			assert(v.y > -1 && v.y < 1);
		}
	}

	void normalize(std::vector<vec4f> &vertices)
	{
		for (auto &v : vertices)
			v /= v.w;
	}

	void test_polygon_clipping()
	{
		printf("Testint polygon clipping...");
		
		std::vector<vec4f> planes;
		// left plane
		planes.push_back({ 1, 0, 0, 1 });
		// top plane
		planes.push_back({ 0, -1, 0, 1 });
		// right plane
		planes.push_back({ -1, 0, 0, 1 });
		// bottom plane
		planes.push_back({ 0, 1, 0, 1 });

		std::vector<vec3f> vertices;

		//-------------------------------------------------------------
		// Fail case
		std::vector<vec3f> fail_cases = {
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
			check_clipping(vertices);
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
				vertices.push_back({ x, y, 0 });
				printf("v%d=(%f, %f) ", i, x, y);
			}
			printf("\n");
			wyc::clip_polygon(planes, vertices);
			check_clipping(vertices);
		}

		printf("Test Ok!\n");
	}

	void test_homogeneous_clipping()
	{
		printf("Testint polygon clipping in homogeneous space...\n");

		std::vector<vec4f> vertices;
		float fov = 45, aspect = 4.0f / 3;
		float n = 1, f = 100;
		vec4f v;
		mat4f proj;
		wyc::set_perspective(proj, fov, aspect, n, f);

		//-------------------------------------------------------------
		// Fail case
		std::vector<vec4f> fail_cases = {
		};
		for (int i = 2, c = fail_cases.size(); i < c; i += 3)
		{
			vertices.clear();
			vertices.push_back(fail_cases[i - 2]);
			vertices.push_back(fail_cases[i - 1]);
			vertices.push_back(fail_cases[i]);
			wyc::clip_polygon(vertices);
			normalize(vertices);
			check_clipping_strict(vertices);
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
			normalize(vertices);
			check_clipping_strict(vertices);
		}
	}

	void test_matrix()
	{
		printf("testing matrix22...");
		wyc::test_matrix22<float>();
		printf("Ok\n");
		printf("testing matrix33...");
		wyc::test_matrix33<float>();
		printf("Ok\n");
		printf("testing matrix44...");
		wyc::test_matrix44<float>();
		printf("Ok\n");
		printf("testing matrix55...");
		wyc::test_matrix55<float>();
		printf("Ok\n");
	}

} // namespace test