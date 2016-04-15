#pragma once
#include <string>
#include <OpenEXR/ImathMatrix.h>
#include "shader.h"

#define PROPERTY
#define PUBLIC_PROPERTY

namespace wyc
{
	class CMaterial
	{
	public:
		CMaterial();
		virtual ~CMaterial() {}
		virtual bool bind_vertex(const CVertexBuffer &vb) = 0;
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const = 0;
		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const = 0;

	public:
		// public material property
		std::string name;
		shader_ptr shader;
		Matrix44f mvp_matrix;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

} // namespace wyc