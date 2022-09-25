#include <unordered_map>
#include "renderer/sparrow/spw_pipeline2.h"

namespace wyc
{

CSpwTilePipeline::CSpwTilePipeline()
{
	
}

CSpwTilePipeline::~CSpwTilePipeline()
{
}


void CSpwTilePipeline::draw(const CMesh *mesh, const CMaterial *material)
{
	auto &vb = mesh->vertex_buffer();
	auto &ib = mesh->index_buffer();
	
	unsigned vertex_per_prim;
	if(mesh->primitive_type() == PRIM_TYPE_TRIANGLE)
	{
		vertex_per_prim = 3;
	}
	else {
		// unsupported primitive 
		return;
	}
	
	unsigned prim_count = (unsigned)ib.size() / vertex_per_prim;
	unsigned vert_count = prim_count * vertex_per_prim;
	unsigned max_prim_per_batch = 12000;
	unsigned prim_per_batch = std::min(prim_count, max_prim_per_batch);
	
	auto *index_data = ib.data();
	auto *vertex_data = vb.get_buffer();
	int stride = vb.vertex_component();
	const float *pos_stream = (const float*)vb.attrib_stream(ATTR_POSITION);
	
	// split by batch
	for(unsigned prim_index = 0; prim_index < prim_count; prim_index += prim_per_batch)
	{
		// draw primitives
		for(unsigned i = prim_index; i < prim_per_batch; ++i)
		{
			auto i1 = *index_data++;
			auto i2 = *index_data++;
			auto i3 = *index_data++;
			
			const float *v1 = pos_stream + i1 * stride;
			const float *v2 = pos_stream + i2 * stride;
			const float *v3 = pos_stream + i3 * stride;
			
			
		} // loop of primitives
	} // loop of batch
}

} // namespace wyc
