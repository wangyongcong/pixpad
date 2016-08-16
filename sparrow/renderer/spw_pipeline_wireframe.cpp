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
	CSpwPlotter plt(m_rt.get(), task, Imath::V2i(0, 0));
	const auto &block = task.block;
	Imath::Box2f clip_window = {
		{0.5f, 0.5f}, {block.max.x - block.min.x - 0.5f, block.max.y - block.min.y - 0.5f}
	};
	unsigned w = m_rt->width(), h = m_rt->height();
	for (size_t j = 2; j < count; ++j)
	{
		// backface culling
		Imath::V2f v10(p0->x - p1->x, p0->y - p1->y);
		Imath::V2f v12(p2->x - p1->x, p2->y - p1->y);
		if (v10.cross(v12) * m_clock_wise > 0) {
			// clip & draw line
			v10 = { p0->x, p0->y };
			v12 = { p1->x, p1->y };
			if (clip_line(v10, v12, clip_window))
				draw_line(plt, v10, v12);
			v10 = { p1->x, p1->y };
			v12 = { p2->x, p2->y };
			if (clip_line(v10, v12, clip_window))
				draw_line(plt, v10, v12);
			v10 = { p2->x, p2->y };
			v12 = { p0->x, p0->y };
			if (clip_line(v10, v12, clip_window))
				draw_line(plt, v10, v12);
		}
		// next one
		p1 = p2;
		p2 = (Vec4f*)(((float*)p2) + task.out_stride);
	}
}
