#include "tex_color.h"
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>

namespace wyc 
{
	void CShaderTexColor::vertex_shader(const float *vertex_in, float *vertex_out, Imath::Vec4<float> &clip_pos) const
	{
		const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
		VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
		Imath::V4f pos(in->pos);
		pos = m_uniform.mvp * pos;
		clip_pos = pos;
		out->uv = in->uv;
		out->normal = in->normal;
	}

	bool CShaderTexColor::fragment_shader(const float *vertex_out, Imath::Color4<float> &frag_color) const
	{
		const VertexOut* vout = reinterpret_cast<const VertexOut*>(vertex_out);
		frag_color = { m_uniform.color.r, m_uniform.color.g, m_uniform.color.b, m_uniform.color.a };
		return true;
	}

	size_t CShaderTexColor::get_vertex_stride() const
	{
		return VertexOut::Layout::component;
	}

	size_t CShaderTexColor::get_vertex_size() const
	{
		return sizeof(VertexOut);
	}

} // namespace wyc