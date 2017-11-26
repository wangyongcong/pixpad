#pragma once
#include <algorithm>
#include "ImathMatrix.h"
#include "material.h"


class CMaterialWireframe : public wyc::CMaterial
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
		UNIFORM_SLOT(Imath::C4f, fill_color)
	UNIFORM_MAP_END

public:
	CMaterialWireframe()
		: CMaterial("Wireframe")
		, color(1.0f, 1.0f, 1.0f, 1.0f)
		, fill_color(0.1f, 0.1f, 0.1f, 0.1f)
	{
		proj_from_world.makeIdentity();
	}

	struct VertexIn {
		const Imath::V3f *pos;
	};

	struct VertexOut {
		Imath::V4f pos;
	};

	// shader interface
	virtual void vertex_shader(const void *vertex_in, void *vertex_out, wyc::CShaderContext *ctx) const override
	{
		const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
		VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
		Imath::V4f pos(*in->pos);
		out->pos = proj_from_world * pos;
	}

	virtual bool fragment_shader(const void *frag_in, Imath::C4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		float dw1dx = (*ctx->w1)[1] - (*ctx->w1)[0];
		float dw2dx = (*ctx->w2)[1] - (*ctx->w2)[0];
		float dw3dx = (*ctx->w3)[1] - (*ctx->w3)[0];
		float dw1dy = (*ctx->w1)[2] - (*ctx->w1)[0];
		float dw2dy = (*ctx->w2)[2] - (*ctx->w2)[0];
		float dw3dy = (*ctx->w3)[2] - (*ctx->w3)[0];

		Imath::V3f d = {
			std::fabs(dw1dx) + std::fabs(dw1dy),
			std::fabs(dw2dx) + std::fabs(dw2dy),
			std::fabs(dw3dx) + std::fabs(dw3dy),
		};
		Imath::V3f w = {(*ctx->w1)[ctx->index], (*ctx->w2)[ctx->index], (*ctx->w3)[ctx->index] };
		auto dist = Imath::V3f{ w.x / d.x, w.y / d.y, w.z / d.z };
		if (dist.x > 1)
			dist.x = 1;
		if (dist.y > 1)
			dist.y = 1;
		if (dist.z > 1)
			dist.z = 1;
		if (dist.x < 0)
			dist.x = 0;
		if (dist.y < 0)
			dist.y = 0;
		if (dist.z < 0)
			dist.z = 0;
		dist = dist * dist * (Imath::V3f(3.0f) - 2.0f * dist);
		float t = std::min({ dist.x, dist.y, dist.z });
		frag_color = color * (1 - t) + t * fill_color;
		return true;
	}

protected:
	Imath::M44f proj_from_world;
	Imath::C4f color;
	Imath::C4f fill_color;
};

