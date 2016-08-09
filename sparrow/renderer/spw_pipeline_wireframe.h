#pragma once
#include "spw_pipeline.h"

namespace wyc
{
	class CSpwPipelineWireFrame : public CSpwPipeline
	{
		typedef CSpwPipeline BaseType;
	public:
		CSpwPipelineWireFrame();
		virtual ~CSpwPipelineWireFrame();
		virtual void feed(const CMesh *mesh, const CMaterial *material);

	protected:
		virtual void draw_triangles(float *vertices, size_t count, RasterTask &task) const;
	};

} // namespace wyc