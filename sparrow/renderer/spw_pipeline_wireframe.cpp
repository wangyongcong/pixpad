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
	process(mesh, material);
}

void wyc::CSpwPipelineWireFrame::draw_triangles(const std::vector<float>& vertices, const std::vector<unsigned>& indices, unsigned stride, CTile & tile) const
{
	const auto &b = tile.bounding;
	Imath::Box2f clip_window = {
		{ 0.5f, 0.5f },{ b.max.x - b.min.x - 0.5f, b.max.y - b.min.y - 0.5f }
	};
	unsigned w = m_rt->width(), h = m_rt->height();

	const float* vec = vertices.data();
	const float* end = vec + vertices.size();
	const Imath::V4f *p0 = (const Imath::V4f*)vec;
	vec += stride;
	const Imath::V4f *p1 = (const Imath::V4f*)vec;
	vec += stride;
	const Imath::V4f *p2 = (const Imath::V4f*)vec;
	Imath::V2f v10, v12;
	while (vec < end)
	{
		// clip & draw line
		v10 = { p0->x, p0->y };
		v12 = { p1->x, p1->y };
		if (clip_line(v10, v12, clip_window))
			draw_line(tile, v10, v12);
		v10 = { p1->x, p1->y };
		v12 = { p2->x, p2->y };
		if (clip_line(v10, v12, clip_window))
			draw_line(tile, v10, v12);
		v10 = { p2->x, p2->y };
		v12 = { p0->x, p0->y };
		if (clip_line(v10, v12, clip_window))
			draw_line(tile, v10, v12);
		// next one
		vec += stride;
		p1 = p2;
		p2 = (const Imath::V4f*)vec;
	} // index loop
}
