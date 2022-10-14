#pragma once
#include "ImathMatrix.h"
#include "renderer/sparrow/material.h"


class CMaterialColor : public wyc::CMaterial
{
	INPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 3)
		INPUT_ATTRIBUTE_LIST_END
	};

	OUTPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 4)
		OUTPUT_ATTRIBUTE_LIST_END
	};

	UNIFORM_MAP{
		UNIFORM_SLOT(wyc::mat4f, proj_from_world)
		UNIFORM_SLOT(wyc::color4f, color)
		UNIFORM_MAP_END
	};

public:
	CMaterialColor()
		: CMaterial("Color")
	{
		proj_from_world.makeIdentity();
		color.setValue(1.0f, 1.0f, 1.0f, 1.0f);
	}
		
	struct VertexIn {
		const wyc::vec3f *pos;
	};

	struct VertexOut {
		wyc::vec4f pos;
	};

	// shader interface
	virtual void vertex_shader(const void *vertex_in, void *vertex_out, wyc::CShaderContext *ctx) const override
	{
		const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
		VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
		wyc::vec4f pos(*in->pos);
		out->pos = proj_from_world * pos;
	}

	virtual bool fragment_shader(const void *frag_in, wyc::color4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		frag_color = color;
		return true;
	}

protected:
	wyc::mat4f proj_from_world;
	wyc::color4f color;
};

