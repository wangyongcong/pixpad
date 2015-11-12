#include <cassert>
#include <numeric>
#include "vector.h"

using namespace wyc;

namespace test
{
	template<typename T>
	void vector_basic_test()
	{
		typedef typename T::scalar_t scalar_t;
		const scalar_t pi = scalar_t(3.1415926), e = scalar_t(2.718281828);
		T v1;
		v1.zero();
		v1 = e;
		v1 = { pi, e };
		T v2 = { scalar_t(1.414), scalar_t(1.736) };
		v1 += v2;
		v1 -= v2;
		v1 *= v2;
		v1 /= v2;
		T v3;
		v3 = v1 + v2;
		v3 = v1 - v2;
		v3 = v1 * v2;
		v3 = v2 / v2;
		v1 += pi;
		v1 -= pi;
		v1 *= pi;
		v1 /= pi;
		v3 = v1 * pi;
		v3 = v1 / pi;
		v3 = pi * v1;
		v2 += e;
		v2.reverse();
		v2.reciprocal();
		scalar_t len;
		len = v2.length2();
		len = v2.length();
		if (len)
			v2.normalize();
		v1.dot(v2);
		v1.cross(v2);
		bool b;
		b = v1 == v2;
		b = v1 != v2;
		b = v1 < v2;
		b = v1 > v2;
		b = v1 <= v2;
		b = v1 >= v2;
	}

	void test_vector()
	{
		printf("testing vec2f...");
		vector_basic_test<xvector<float, 2>>();
		printf("Ok\n");
		printf("testing vec3f...");
		vector_basic_test<xvector<float, 3>>();
		printf("Ok\n");
		printf("testing vec4f...");
		vector_basic_test<xvector<float, 4>>();
		printf("Ok\n");
		printf("testing vec5f...");
		vector_basic_test<xvector<float, 5>>();
		printf("Ok\n");
	}

} // namespace test