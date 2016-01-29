#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <vertex_layout.h>
#include <sparrow/spw_shader.h>

namespace wyc
{
	class CShaderFlatColor : public IShaderProgram
	{
	public:
		typedef VertexP3C3 VertexIn;
		typedef VertexP3C3 VertexOut;

		struct Uniform
		{
			Imath::M44f mvp;
		} m_uniform;

		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Imath::Vec4<float> &clip_pos) const;
		virtual bool fragment_shader(const float *vertex_out, Imath::Color4<float> &frag_color) const;
		virtual size_t get_vertex_stride() const override;
		virtual size_t get_vertex_size() const override;
	};

} // namespace