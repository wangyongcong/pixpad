#pragma once
#include "OpenEXR/ImathMatrix.h"
#include "ImathMatrixExt.h"
#include "material.h"
#include "sampler.h"

namespace wyc
{
	class CMaterialDiffuse : public CMaterial
	{
		INPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 3)
			ATTRIBUTE_SLOT(ATTR_COLOR, 4)
			ATTRIBUTE_SLOT(ATTR_UV0, 2)
		INPUT_ATTRIBUTE_LIST_END

		OUTPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 4)
			ATTRIBUTE_SLOT(ATTR_COLOR, 4)
			ATTRIBUTE_SLOT(ATTR_UV0, 2)
		OUTPUT_ATTRIBUTE_LIST_END

		UNIFORM_MAP
			UNIFORM_SLOT(Imath::M44f, proj_from_world)
			UNIFORM_SLOT(CSampler*, diffuse)
		UNIFORM_MAP_END

	public:
		CMaterialDiffuse()
			: CMaterial("Diffuse")
			, diffuse(nullptr)
		{

		}

		struct VertexIn
		{
			const Imath::V3f *pos;
			const Imath::C4f *color;
			const Imath::V2f *uv;
		};

		struct VertexOut
		{
			Imath::V4f pos;
			Imath::C4f color;
			Imath::V2f uv;
		};

		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			auto in = reinterpret_cast<const VertexIn*>(vertex_in);
			auto out = reinterpret_cast<VertexOut*>(vertex_out);
			Imath::V4f pos(*in->pos);
			out->pos = proj_from_world * pos;
			out->color = *in->color;
			out->uv = *in->uv;
		}

		virtual bool fragment_shader(const void *frag_in, Imath::C4f &frag_color) const override
		{
			auto in = reinterpret_cast<const VertexOut*>(frag_in);
			Imath::C4f diffuse_color;
			diffuse->sample2d(in->uv, diffuse_color);
			frag_color = diffuse_color * in->color;
			return true;
		}

	private:
		Imath::M44f proj_from_world;
		CSampler *diffuse;
	};


} // namespace wyc