#pragma once
#include <algorithm>
#include "ImathMatrix.h"
#include "material.h"
#include "shader_api.h"

class CMaterialLambert : public wyc::CMaterial
{
	INPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 3)
		ATTRIBUTE_SLOT(wyc::ATTR_NORMAL, 3)
		INPUT_ATTRIBUTE_LIST_END
	};

	OUTPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 4)
		ATTRIBUTE_SLOT(wyc::ATTR_USAGE_0, 3)
		ATTRIBUTE_SLOT(wyc::ATTR_NORMAL, 3)
		OUTPUT_ATTRIBUTE_LIST_END
	};

	UNIFORM_MAP{
		UNIFORM_SLOT(Imath::M44f, proj_from_world)
		UNIFORM_SLOT(Imath::M44f, view_from_world)
		UNIFORM_SLOT(Imath::M44f, normal_transform)
		UNIFORM_SLOT(Imath::V3f, light_pos_view)
		UNIFORM_MAP_END
	};

	struct VertexIn {
		const Imath::V3f *pos;
		const Imath::V3f *normal;
	};

	struct VertexOut {
		Imath::V4f pos;
		Imath::V3f surface_pos;
		Imath::V3f surface_normal;
	};

public:
	virtual void vertex_shader(const void *vertex_in, void *vertex_out, wyc::CShaderContext *ctx) const override
	{
		const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
		VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
		Imath::V4f pos(*in->pos);
		out->pos = proj_from_world * pos;
		out->surface_pos = view_from_world * (*in->pos);
		out->surface_normal = normal_transform * (*in->normal);
	}

	virtual bool fragment_shader(const void *frag_in, Imath::C4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		auto in = reinterpret_cast<const VertexOut*>(frag_in);
		auto l = light_pos_view - in->surface_pos;
		l.normalize();
		auto c = l.dot(in->surface_normal);
		c = std::max(0.0f, c);
		frag_color.setValue(c, c, c, 1.0f);
		return true;
	}

protected:
	Imath::M44f proj_from_world;
	Imath::M44f view_from_world;
	Imath::M44f normal_transform;
	Imath::V3f light_pos_view;
};