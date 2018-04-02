#pragma once
#include "ImathMatrix.h"
#include "material.h"
#include "sampler.h"

class CMaterialDiffuse : public wyc::CMaterial
{
	INPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 3)
		ATTRIBUTE_SLOT(wyc::ATTR_COLOR, 4)
		ATTRIBUTE_SLOT(wyc::ATTR_UV0, 2)
		INPUT_ATTRIBUTE_LIST_END
	};

	OUTPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 4)
		ATTRIBUTE_SLOT(wyc::ATTR_COLOR, 4)
		ATTRIBUTE_SLOT(wyc::ATTR_UV0, 2)
		OUTPUT_ATTRIBUTE_LIST_END
	};

	UNIFORM_MAP{
		UNIFORM_SLOT(wyc::mat4f, proj_from_world)
		UNIFORM_SLOT(wyc::CSampler*, diffuse)
		UNIFORM_MAP_END
	};

public:
	CMaterialDiffuse()
		: CMaterial("Diffuse")
		, diffuse(nullptr)
	{

	}

	struct VertexIn
	{
		const wyc::vec3f *pos;
		const wyc::color4f *color;
		const wyc::vec2f *uv;
	};

	struct VertexOut
	{
		wyc::vec4f pos;
		wyc::color4f color;
		wyc::vec2f uv;
	};

	virtual void vertex_shader(const void *vertex_in, void *vertex_out, wyc::CShaderContext *ctx) const override
	{
		auto in = reinterpret_cast<const VertexIn*>(vertex_in);
		auto out = reinterpret_cast<VertexOut*>(vertex_out);
		wyc::vec4f pos(*in->pos);
		out->pos = proj_from_world * pos;
		out->color = *in->color;
		out->uv = *in->uv;
	}

	virtual bool fragment_shader(const void *frag_in, wyc::color4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		auto in = reinterpret_cast<const VertexOut*>(frag_in);
		wyc::color4f diffuse_color;
		diffuse->sample2d(in->uv, diffuse_color);
		frag_color = diffuse_color * in->color;
		return true;
	}

protected:
	wyc::mat4f proj_from_world;
	wyc::CSampler *diffuse;
};

