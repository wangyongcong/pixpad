#include <cstdlib>
#include <cstdio>

#include "vecmath.h"

using namespace wyc;

int main(int argv, char *argc[])
{
	printf("Test libmathex\n");

	// test vector
	printf("vector2...\n");
	vector_test<xvec2f_t>();
	printf("vector3...\n");
	vector_test<xvec3f_t>();
	printf("vector4...\n");
	vector_test<xvec4f_t>();
	printf("vector8...\n");
	vector_test<xvector<float, 8>>();

	// test matrix
	printf("matrix22...\n");
	matrix_test<xmat2f_t>();
	printf("matrix33...\n");
	matrix_test<xmat3f_t>();
	printf("matrix44...\n");
	matrix_test<xmat4f_t>();

	printf("Test Ok!\n");
	getchar();
}