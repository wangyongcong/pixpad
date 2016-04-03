#pragma once
#include <memory>
#include <mathex/mathfwd.h>
#include "vertex_buffer.h"

namespace wyc
{
	class IShaderProgram
	{
	public:
		virtual bool bind_vertex(const CVertexBuffer &vb) = 0;
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const = 0;
		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const = 0;
		virtual size_t get_vertex_stride() const = 0;
		virtual size_t get_vertex_size() const = 0;
	};

	typedef std::shared_ptr<IShaderProgram> shader_ptr;

} // namespace wyc