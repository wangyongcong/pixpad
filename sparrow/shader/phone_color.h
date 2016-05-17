#pragma once
#include <OpenEXR/ImathMatrix.h>
#include "vertex_layout.h"
#include "material.h"
#include "mathfwd.h"
#include "ImathMatrixExt.h"

namespace wyc
{
	class CMaterialPhoneColor : public CMaterial
	{
	public:
		struct VertexIn {
			const Vec3f *pos;
			const Color4f *color;
		};

		struct VertexOut {
			Vec4f pos;
			Color4f color;
		};

		virtual const AttribDefine& get_attrib_define() const override
		{
			static AttribSlot ls_in_attribs[] = {
				{ ATTR_POSITION, 3 },
				{ ATTR_COLOR, 4 },
			};
			static AttribSlot ls_out_attirbs[] = {
				{ ATTR_POSITION, 4 },
				{ ATTR_COLOR, 4 },
			};
			static AttribDefine ls_attrib_define = {
				ls_in_attribs, // attribute slot binding
				2, // attribute count
				7, // total attribute components
				ls_out_attirbs,
				2,
				8,
			};
			return ls_attrib_define;
		}

		virtual const std::unordered_map<std::string, CUniform>& get_uniform_define() const override
		{
			DECLARE_UNIFORM_MAP(CMaterialPhoneColor) {
				MAKE_UNIFORM(Matrix44f, mvp_matrix)
			};
			return UNIFORM_MAP;
		}

		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			VertexIn *in = (VertexIn*)vertex_in;
			Vec4f pos(*in->pos);
			pos.z = -1.0f;
			pos = this->mvp_matrix * pos;
			VertexOut *out = (VertexOut*)vertex_out;
			out->pos = pos;
			out->color = *in->color;
		}

		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const override
		{
			const VertexOut* out = reinterpret_cast<const VertexOut*>(vertex_out);
			frag_color = out->color;
			return true;
		}

	protected:
		Matrix44f mvp_matrix;
	};

} // namespace