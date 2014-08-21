#include <iostream>
#include <atomic>

using namespace std;

class CTest
{
public:
	CTest(int count)
	{
		if(count > 0) {
			m_ptr = new int[count];
			memset(m_ptr, 0, sizeof(int)*count);
			m_count = count;
		}
		else {
			m_ptr = 0;
			m_count = 0;
		}
	}
	CTest(CTest &&t)
	{
		if(m_ptr)
			delete [] m_ptr;
		m_ptr = t.m_ptr;
		m_count = t.m_count;
		t.m_ptr = 0;
		t.m_count = 0;
		printf("CTest move constructor\n");
	}
	CTest(const CTest &t)
	{
		if(m_ptr)
			delete [] m_ptr;
		m_count = t.m_count;
		if(m_count > 0)
		{
			m_ptr = new int[m_count];
			memcpy(m_ptr, t.m_ptr, sizeof(int)*m_count);
		}
		printf("CTest copy constructor\n");
	}
	~CTest()
	{
		if(m_ptr) {
			delete [] m_ptr;
			m_ptr = 0;
		}
	}

private:
	int *m_ptr;
	unsigned m_count;
};

#include "raster.h"

void test_triangle_sampler()
{
	using namespace wyc;
	xsurface surf;
	xvec2f_t center = {0.5f, 0.5f};
	xvec2f_t verts[3];
	float width = 64, height = 64, x, y;
	for(int i=0; i<3; ++i)
	{
		x = width * float(rand()) / RAND_MAX;
		y = height * float(rand()) / RAND_MAX;
		verts[i].set(x, y);
	}
	sample_triangle(center, verts, surf);
	printf("Surface detail:\n");
	printf("(%f, %f), (%f, %f), (%f, %f)",
		verts[0].x, verts[0].y, verts[1].x, verts[1].y, verts[2].x, verts[2].y);
	surf.detail();
}

int main()
{
	test_triangle_sampler();
	return 0;
}

