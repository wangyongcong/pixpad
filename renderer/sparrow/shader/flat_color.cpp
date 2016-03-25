#include "flat_color.h"
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>

namespace wyc 
{
	//void CShaderFlatColor::vertex_shader(const float *vertex_in, float *vertex_out, Imath::Vec4<float> &clip_pos) const
	//{
	//	const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
	//	VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
	//	Imath::V4f pos(in->pos);
	//	pos.z = -1.0f;
	//	pos = m_uniform.mvp * pos;
	//	clip_pos = pos;
	//	out->pos = in->pos;
	//	out->color = in->color;
	//}

	//bool CShaderFlatColor::fragment_shader(const float *vertex_out, Imath::Color4<float> &frag_color) const
	//{
	//	const VertexOut* vout = reinterpret_cast<const VertexOut*>(vertex_out);
	//	frag_color = { vout->color.x, vout->color.y, vout->color.z, 1.0f };
	//	return true;
	//}

	//size_t CShaderFlatColor::get_vertex_stride() const
	//{
	//	return VertexOut::Layout::component;
	//}

	//size_t CShaderFlatColor::get_vertex_size() const
	//{
	//	return sizeof(VertexOut);
	//}

} // namespace wyc