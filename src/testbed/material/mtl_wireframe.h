#pragma once
#include <algorithm>
#include <ImathMatrix.h>
#include "vecmath.h"
#include "material.h"
#include "shader_api.h"

class CMaterialWireframe : public wyc::CMaterial
{
	INPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 3)
		INPUT_ATTRIBUTE_LIST_END
	};

	OUTPUT_ATTRIBUTE_LIST{
		ATTRIBUTE_SLOT(wyc::ATTR_POSITION, 4)
		ATTRIBUTE_SLOT(wyc::ATTR_UV0, 3)
		OUTPUT_ATTRIBUTE_LIST_END
	};

	UNIFORM_MAP{
		UNIFORM_SLOT(wyc::mat4f, proj_from_world)
		UNIFORM_SLOT(wyc::color4f, line_color)
		UNIFORM_SLOT(wyc::color4f, fill_color)
		UNIFORM_MAP_END
	};


public:
	CMaterialWireframe()
		: CMaterial("Wireframe")
		, line_color(0.0f, 0.0f, 0.0f, 1.0f)
		, fill_color(1.0f, 1.0f, 1.0f, 1.0f)
		, line_width(1.5f)
	{
		proj_from_world.makeIdentity();
		m_sample_data.fill(0);
		unsigned len = (unsigned)m_sample_data.size();
		assert((len & (len - 1)) == 0);
		m_sample_data[len - 1] = 1.0f;
		m_sample_data[len - 2] = 0.5f;

		float *ptr = &m_sample_data[0];
		for (auto i = 0u; i < line_sample.size(); ++i) {
			line_sample[i] = { ptr, len };
			auto half = len >> 1;
			ptr += half;
			len = half;
		}

		m_feature |= wyc::MF_NO_PERSPECTIVE_CORRECTION;
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

	virtual void geometry_shader(void *triangles) const override
	{
		struct Vertex {
			wyc::vec4f pos;
			wyc::vec3f uv;
		};
		auto verts = reinterpret_cast<Vertex*>(triangles);
		verts[0].uv.setValue(0.0f, 1.0f, 1.0f);
		verts[1].uv.setValue(1.0f, 0.0f, 1.0f);
		verts[2].uv.setValue(1.0f, 1.0f, 0.0f);
	}

	virtual bool fragment_shader(const void *frag_in, wyc::color4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		struct Vertex {
			wyc::vec4f pos;
			wyc::vec3f uv;
		};
		auto in = reinterpret_cast<const Vertex*>(frag_in);
		auto du_dx = ctx->ddx(&Vertex::uv);
		auto du_dy = ctx->ddy(&Vertex::uv);
		auto texture_size = m_sample_data.size();
		float a[3], f;
		for (int i = 0; i < 3; ++i) {
			int lvl = wyc::mipmap_level(du_dx[i], du_dy[i], texture_size, f);
			if (lvl > 0) {
				float c1 = sample_alpha(in->uv[i], lvl);
				float c2 = sample_alpha(in->uv[i], lvl - 1);
				a[i] = c1 * f + c2 * (1 - f);
			}
			else {
				a[i] = sample_alpha(in->uv[i]);
			}
		}
		float t = std::max({ a[0], a[1], a[2] });
		frag_color = fill_color * (1 - t) + line_color * t;
		return true;
	}

	float sample_alpha(float s, unsigned level=0) const {
		SampleData sample;
		if (level >= line_sample.size()) {
			sample = line_sample.back();
		}
		else {
			sample = line_sample[level];
		}
		s *= sample.size;
		float t = std::floor(s);
		auto ll = unsigned(t);
		auto rr = ll + 1;
		if (rr >= sample.size) {
			return sample.data[sample.size - 1];
		}
		t = s - t;
		return sample.data[ll] * (1 - t) + sample.data[rr] * t;
	}

protected:
	wyc::mat4f proj_from_world;
	wyc::color4f line_color;
	wyc::color4f fill_color;
	float line_width;
	constexpr static int SAMPLE_LEVEL = 11;
	constexpr static int TEXTURE_SIZE = 1 << (SAMPLE_LEVEL - 1);
	std::array<float, TEXTURE_SIZE> m_sample_data;
	struct SampleData {
		const float* data;
		unsigned size;
	};
	std::array<SampleData, SAMPLE_LEVEL> line_sample;
};

