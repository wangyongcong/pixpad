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
		, line_width(1.5f)
	{
		proj_from_world.makeIdentity();
		line_sample.assign(0);
		auto len = line_sample.size();
		line_sample[len - 1] = 1.0f;
		line_sample[len - 2] = 0.5f;
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

		auto duvdx = ctx->ddx(&Vertex::uv);
		auto duvdy = ctx->ddy(&Vertex::uv);
		union {
			float f;
			unsigned i;
		} j;
		auto texture_size = line_sample.size();
		float a[3];
		for (int i = 0; i < 3; ++i) {
			j.f = std::max(std::fabs(duvdx[i]), std::fabs(duvdy[i])) * texture_size;
			int e = (j.i & 0x7f800000) >> 23;
			if (e > 127) {
				e -= 127;
				int m = j.i & 0x7fffff;
				float f = float(m) / (1 << 23);
				float c1 = sample_alpha(in->uv[i], e);
				float c2 = sample_alpha(in->uv[i], e - 1);
				a[i] = c1 * f + c2 * (1 - f);
			}
			else {
				a[i] = sample_alpha(in->uv[i]);
			}
		}
		frag_color = fill_color;
		//for (int i = 0; i < 3; ++i) {
		//	float t = a[i];
		//	//t = smooth_step(t);
		//	frag_color = frag_color * (1 - t) + line_color * t;
		//}
		float t = std::max({ a[0], a[1], a[2] });
		//t = smooth_step(t);
		frag_color = frag_color * (1 - t) + line_color * t;
		return true;
	}

	float sample_alpha(float s, unsigned level=0) const {
		auto len = line_sample.size();
		auto ll = 0u;
		while (level > 0) {
			level -= 1;
			ll += len >> 1;
			len >>= 1;
		}
		s *= len;
		float t = std::floor(s);
		ll += int(t);
		if (ll >= line_sample.size() - 1) {
			return line_sample.back();
		}
		unsigned rr = ll + 1;
		t = s - t;
		return line_sample[ll] * (1 - t) + line_sample[rr] * t;
	}

	float smooth_step(float x) const {
		auto t = x / line_width;
		if (t > 1.0f)
			t = 1.0f;
		return t * t * (3.0f - 2.0f * t);
	}

protected:
	Imath::M44f proj_from_world;
	Imath::C4f line_color;
	Imath::C4f fill_color;
	std::array<float, 1024> line_sample;
	float line_width;
};

