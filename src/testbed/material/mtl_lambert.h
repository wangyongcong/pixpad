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
		UNIFORM_SLOT(wyc::mat4f, proj_from_world)
		UNIFORM_SLOT(wyc::mat4f, view_from_world)
		UNIFORM_SLOT(wyc::mat4f, normal_transform)
		UNIFORM_SLOT(wyc::vec3f, light_pos_view)
		UNIFORM_MAP_END
	};

	struct VertexIn {
		const wyc::vec3f *pos;
		const wyc::vec3f *normal;
	};

	struct VertexOut {
		wyc::vec4f pos;
		wyc::vec3f surface_pos;
		wyc::vec3f surface_normal;
	};

public:
	virtual void vertex_shader(const void *vertex_in, void *vertex_out, wyc::CShaderContext *ctx) const override
	{
		const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
		VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
		wyc::vec4f pos(*in->pos);
		out->pos = proj_from_world * pos;
		out->surface_pos = view_from_world * (*in->pos);
		out->surface_normal = normal_transform * (*in->normal);
	}

	virtual bool fragment_shader(const void *frag_in, wyc::color4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		auto in = reinterpret_cast<const VertexOut*>(frag_in);
		auto l = light_pos_view - in->surface_pos;
		l.normalize();
		auto c = l.dot(in->surface_normal);
		c = std::max(0.0f, c);
		//c = std::pow(c, 1 / 2.2f);
		frag_color.setValue(c, c, c, 1.0f);
		return true;
	}

protected:
	wyc::mat4f proj_from_world;
	wyc::mat4f view_from_world;
	wyc::mat4f normal_transform;
	wyc::vec3f light_pos_view;
};