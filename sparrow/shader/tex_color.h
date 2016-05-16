#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <vertex_layout.h>
#include <shader.h>
#include <sparrow/spw_texture.h>

namespace wyc
{
	class CShaderTexColor : public IShaderProgram
	{
	public:
		typedef VertexP3Tex2N3 VertexIn;
		typedef VertexTex2N3 VertexOut;

		struct Uniform
		{
			Imath::M44f mvp;
			Imath::C4f color;
			CSpwTexture2D tex2d;
		} m_uniform;

		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Imath::Vec4<float> &clip_pos) const;
		virtual bool fragment_shader(const float *vertex_out, Imath::Color4<float> &frag_color) const;
		virtual size_t get_vertex_stride() const override;
		virtual size_t get_vertex_size() const override;
	};

} // namespace