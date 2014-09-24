#include <cstdlib>
#include <cstdio>

#include "vecmath.h"

using namespace wyc;

int main(int argv, char *argc[])
{
	printf("Test libmathex\n");

	xvec3f_t v1 = { 0, 0 };
	xvec3f_t v2 = { 1, 2, 3 };
	assert(v1.x == 0 && v1.y == 0);
	assert(v2.x == 1 && v2.y == 2 && v2.z == 3);
	float t;
	t = v1.dot(v2);
	v1.cross(v2);
	v1[0];
	v1[1] = 3;
	v1[2] = 4;
	v1 += v2;
	v1 -= v2;
	v1 *= v2;
	v1 *= t;
	v1 /= v2;
	v1 /= t;
	v1 = -v2;
	v1.reverse();

	getchar();
}