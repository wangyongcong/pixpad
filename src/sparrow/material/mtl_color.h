#pragma once
#include "ImathMatrix.h"
#include "material.h"

namespace wyc
{
	template<EAttribUsage U, int C>
	struct VTypetrait
	{
	};

	template<> struct VTypetrait<ATTR_POSITION, 3>
	{
	public:
		typedef Imath::V3f vertex_t;
	};

	class CMaterialColor : public CMaterial
	{
		INPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 3)
		INPUT_ATTRIBUTE_LIST_END

		OUTPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 4)
		OUTPUT_ATTRIBUTE_LIST_END

		UNIFORM_MAP
			UNIFORM_SLOT(Imath::M44f, proj_from_world)
			UNIFORM_SLOT(Imath::C4f, color)
		UNIFORM_MAP_END

	public:
		CMaterialColor()
			: CMaterial("Color")
		{
			proj_from_world.makeIdentity();
			color.setValue(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		struct VertexIn {
			const Imath::V3f *pos;
		};

		struct VertexOut {
			Imath::V4f pos;
		};

		// shader interface
		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
			VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
			Imath::V4f pos(*in->pos);
			out->pos = proj_from_world * pos;
		}

		virtual bool fragment_shader(const void *frag_in, Imath::C4f &frag_color) const override
		{
			frag_color = color;
			return true;
		}

	private:
		Imath::M44f proj_from_world;
		Imath::C4f color;
	};

} // namespace