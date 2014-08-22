#include <iostream>
#include <atomic>

#include "raster.h"
#include "test_sampler.h"

using namespace std;

void test_triangle_sampler()
{
	using namespace wyc;
	xsurface surf;
	const xvec2f_t center = {0.5f, 0.5f};
	const float cx = 64, cy = 64, radius = 64;
	const float pi2 = 3.1415926f * 2;
	xvec2f_t verts[3];
	verts[0].set(cx, cy);
	for(int cnt = 0; cnt < 10000; ++cnt)
	{
		for(int i=1; i<3; ++i)
		{
			float a1 = pi2 * float(rand()) / RAND_MAX;
			float dx = radius * std::cos(a1);
			float dy = radius * std::sin(a1);
			verts[i].set(cx + dx, cy + dy);
		}
		sample_triangle(center, verts, surf);
		surf.verify(center, verts);
		surf.clear();
	}
}

int main()
{
	printf("Testint triangle sampler...");
	test_triangle_sampler();
	printf("OK!\n");
	return 0;
}

