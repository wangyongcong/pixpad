#pragma once
#include <string>
#include <vector>
#include <OpenEXR/ImathMatrix.h>
#include "shader.h"

namespace wyc
{
	class CMaterial
	{
	public:
		CMaterial();
		virtual ~CMaterial() {}
		virtual bool bind_vertex(const CVertexBuffer &vb);
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const = 0;
		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const = 0;
		void set_vertex(unsigned idx, const char **ptr) const
		{
			for (const AttribStream &s : m_attrib_stream)
			{
				*ptr++ = s.stream + s.stride * idx;
			}
		}

		struct AttribSlot 
		{
			EAttribUsage usage;
			unsigned char component;
		};
		struct AttribDefine
		{
			AttribSlot *attrib_slots;
			unsigned attrib_count;
			unsigned attrib_component;
		};
		virtual const AttribDefine& get_attrib_define() const
		{
			static AttribDefine ls_attrib_define = {
				0, 0, 0
			};
			return ls_attrib_define;
		}

		struct AttribStream {
			const char *stream;
			size_t stride;
		};
		std::vector<AttribStream> m_attrib_stream;

	public:
		// public material property
		std::string name;
		shader_ptr shader;
		Matrix44f mvp_matrix;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;


} // namespace wyc