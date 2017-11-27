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
		ATTRIBUTE_SLOT(ATTR_UV0, 3)
	OUTPUT_ATTRIBUTE_LIST_END

	UNIFORM_MAP
		UNIFORM_SLOT(Imath::M44f, proj_from_world)
		UNIFORM_SLOT(Imath::C4f, line_color)
		UNIFORM_SLOT(Imath::C4f, fill_color)
	UNIFORM_MAP_END

public:
	CMaterialWireframe()
		: CMaterial("Wireframe")
		, line_color(0.0f, 0.0f, 0.0f, 1.0f)
		, fill_color(1.0f, 1.0f, 1.0f, 1.0f)
	{
		proj_from_world.makeIdentity();
		line_sample.assign(0);
		auto len = line_sample.size();
		float a = 1.0f;
		for (auto i = len - 1; i >= len - 16; --i) {
			line_sample[i] = a;
			a *= 0.7f;
		}
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

	virtual void geometry_shader(void *triangles) const override
	{
		struct Vertex {
			Imath::V4f pos;
			Imath::V3f uv;
		};
		auto verts = reinterpret_cast<Vertex*>(triangles);
		verts[0].uv.setValue(0.0f, 1.0f, 1.0f);
		verts[1].uv.setValue(1.0f, 0.0f, 1.0f);
		verts[2].uv.setValue(1.0f, 1.0f, 0.0f);
	}

	virtual bool fragment_shader(const void *frag_in, Imath::C4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		struct Vertex {
			Imath::V4f pos;
			Imath::V3f uv;
		};
		auto in = reinterpret_cast<const Vertex*>(frag_in);
		float a1 = sample_alpha(in->uv.x);
		float a2 = sample_alpha(in->uv.y);
		float a3 = sample_alpha(in->uv.z);
		frag_color = fill_color;
		float a = std::max({ a1, a2, a3 });
		//float a = a1 + a2 + a3;
		a = smooth_step(a);
		frag_color = frag_color * (1 - a) + line_color * a;
		return true;
	}

	float sample_alpha(float s) const {
		s *= line_sample.size();
		float t = std::floor(s);
		unsigned ll = int(t);
		if (ll >= line_sample.size() - 1) {
			return line_sample.back();
		}
		unsigned rr = ll + 1;
		t = s - t;
		return line_sample[ll] * (1 - t) + line_sample[rr] * t;
	}

	float smooth_step(float x) const {
		constexpr float width = 0.5f;
		auto t = x / width;
		if (t > 1.0f)
			t = 1.0f;
		return t * t * (3.0f - 2.0f * t);
	}

protected:
	Imath::M44f proj_from_world;
	Imath::C4f line_color;
	Imath::C4f fill_color;
	std::array<float, 256> line_sample;
};

