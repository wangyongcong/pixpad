#pragma once
#include <OpenEXR/ImathForward.h>

namespace wyc
{
	class IShaderProgram
	{
	public:
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Imath::Vec4<float> &clip_pos) const = 0;
		virtual bool fragment_shader(const float *vertex_out, Imath::Color4<float> &frag_color) const = 0;
		virtual size_t get_vertex_stride() const = 0;
		virtual size_t get_vertex_size() const = 0;
	};

} // namespace wyc