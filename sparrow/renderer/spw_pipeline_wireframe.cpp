#include "spw_pipeline_wireframe.h"
#include "clipping.h"
#include "spw_rasterizer.h"

wyc::CSpwPipelineWireFrame::CSpwPipelineWireFrame()
{
}

wyc::CSpwPipelineWireFrame::~CSpwPipelineWireFrame()
{
}

void wyc::CSpwPipelineWireFrame::feed(const CMesh * mesh, const CMaterial * material)
{
	BaseType::feed(mesh, material);
}

void wyc::CSpwPipelineWireFrame::draw_triangles(float * vertices, size_t count, RasterTask & task) const
{
	Vec4f *p0 = (Vec4f*)vertices, *p1 = (Vec4f*)(vertices + task.out_stride), *p2 = (Vec4f*)(vertices + task.out_stride * 2);
	Vec2i p0i, p1i, p2i;
	p0i = { fast_floor(p0->x), fast_floor(p0->y) };
	p1i = { fast_floor(p1->x), fast_floor(p1->y) };
	p2i = { fast_floor(p2->x), fast_floor(p2->y) };
	CSpwPlotter plt(m_rt.get(), task, Imath::V2i(0, 0));
	unsigned w = m_rt->width(), h = m_rt->height();
	for (size_t j = 2; j < count; ++j)
	{
		// backface culling
		Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
		Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
		if (v10.cross(v12) * m_clock_wise > 0) {
			// calculate triangle bounding box and intersection of region block
			line_bresenham(plt, p0i.x, p0i.y, p1i.x, p1i.y);
			line_bresenham(plt, p1i.x, p1i.y, p2i.x, p2i.y);
			line_bresenham(plt, p2i.x, p2i.y, p0i.x, p0i.y);
		}
		// next one
		p1 = p2;
		p2 = (Vec4f*)(((float*)p2) + task.out_stride);
		p2i = { fast_floor(p2->x), fast_floor(p2->y) };
	}
}
