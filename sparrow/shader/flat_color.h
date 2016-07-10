#pragma once
#include <OpenEXR/ImathMatrix.h>
#include "vertex_layout.h"
#include "material.h"
#include "ImathMatrixExt.h"

namespace wyc
{
	class CMaterialFlatColor : public CMaterial
	{
	public:
		CMaterialFlatColor() 
			: CMaterial("FlatColor")
		{
			mvp_matrix.makeIdentity();
			color.setValue(1.0f, 1.0f, 1.0f, 1.0f);
		}

		struct VertexIn {
			const Vec3f *pos;
		};

		struct VertexOut {
			Vec4f pos;
		};

		virtual const AttribDefine& get_attrib_define() const override
		{
			static AttribSlot ls_in_attribs[] = {
				{ ATTR_POSITION, 3 },
			};
			static AttribSlot ls_out_attirbs[] = {
				{ ATTR_POSITION, 4 },
			};
			static AttribDefine ls_attrib_define = {
				ls_in_attribs, // attribute slot binding
				1, // attribute count
				3, // total attribute components
				ls_out_attirbs,
				1,
				4,
			};
			return ls_attrib_define;
		}

		virtual const std::unordered_map<std::string, CUniform>& get_uniform_define() const override
		{
			DECLARE_UNIFORM_MAP(CMaterialFlatColor) {
				MAKE_UNIFORM(Matrix44f, mvp_matrix),
				MAKE_UNIFORM(Color4f, color)
			};
			return UNIFORM_MAP;
		}

		// shader interface
		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
			VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
			Vec4f pos(*in->pos);
			log_debug("vert: (%f, %f, %f)", pos.x, pos.y, pos.z);
			out->pos = mvp_matrix * pos;
		}

		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const override
		{
			frag_color = color;
			return true;
		}

	private:
		Matrix44f mvp_matrix;
		Color4f color;
	};

} // namespace