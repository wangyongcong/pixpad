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
		struct AttribSlot
		{
			EAttribUsage usage;
			unsigned char component;
		};
		
		struct AttribDefine
		{
			AttribSlot *attrib_slots;
			unsigned count;
			unsigned component;
		};

		virtual const AttribDefine& get_attrib_define() const
		{
			static AttribDefine ls_attrib_define = {
				0, 0, 0
			};
			return ls_attrib_define;
		}

		CMaterial();
		virtual ~CMaterial() {}
		virtual bool bind_vertex(const CVertexBuffer &vb);
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const = 0;
		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const = 0;

	public:
		// public material property
		std::string name;
		// deprecated
		shader_ptr shader;
		// deprecated
		Matrix44f mvp_matrix;

	protected:
		unsigned calc_attrib_component(const AttribSlot *attrib_slots, unsigned cnt) const
		{
			unsigned size = 0;
			for (auto i = 0; i < cnt; ++i)
			{
				size += attrib_slots[i].component;
			}
			return size;
		}
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

#define MATERIAL_ATTRIB_DEFINE virtual const AttribDefine& get_attrib_define() const { static AttribSlot ls_attrib_slots[] = {
#define MATERIAL_ATTRIB_ENDDEF }; static AttribDefine ls_attrib_define = {\
		ls_attrib_slots,\
		sizeof(ls_attrib_slots) / sizeof(AttribSlot),\
		calc_attrib_component(ls_attrib_slots, sizeof(ls_attrib_slots) / sizeof(AttribSlot))\
	};\
	return ls_attrib_define;}


} // namespace wyc