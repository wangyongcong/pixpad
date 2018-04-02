#include "tile.h"

namespace wyc
{
	CTile::CTile(CSpwRenderTarget *rt, box2i &b, vec2i &c)
		: bounding(b)
		, center(c)
		, m_rt(rt)
		, m_material(nullptr)
		, m_stride(0)
		, m_correction(true)
	{
		m_transform_y = m_rt->height() - center.y - 1;
	}

	void CTile::set_fragment(unsigned vertex_stride, const CMaterial *material) {
		auto buff_size = vertex_stride * 4;
		if (buff_size != m_frag_input.size())
			m_frag_input.resize(buff_size, 0);
		m_frag_interp[0] = &m_frag_input[0];
		m_frag_interp[1] = &m_frag_input[vertex_stride];
		m_frag_interp[2] = &m_frag_input[vertex_stride * 2];
		m_frag_interp[3] = &m_frag_input[vertex_stride * 3];
		m_material = material;
		m_stride = vertex_stride;
		m_ctx.vertex_quad = &m_frag_input[0];

		auto feature = material->feature();
		if (feature & MF_NO_PERSPECTIVE_CORRECTION)
			m_correction = false;
	}

	void CTile::operator() (int x, int y) {
		color4f out_color;
		if (!m_material->fragment_shader(m_frag_input.data(), out_color))
			return;
		out_color.r *= out_color.a;
		out_color.g *= out_color.a;
		out_color.b *= out_color.a;
		y = m_rt->height() - y - 1;
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, out_color);
	}

	void CTile::operator() (int x, int y, float z, float w1, float w2, float w3) {
		x += center.x;
		//y = m_rt->height() - (y + center.y) - 1;
		y = m_transform_y - y;
		// z-test first
		auto &depth = m_rt->get_depth_buffer();
		auto d = *depth.get<float>(x, y);
		if (z >= d)
			return;
		depth.set(x, y, z);
		// interpolate vertex attributes
		const float *i0 = m_v0, *i1 = m_v1, *i2 = m_v2;
		float z_world = 1 / (m_inv_z0 * w1 + m_inv_z1 * w2 + m_inv_z2 * w3);
		//for(float *out = m_fragment_input.data(), *end = out + m_fragment_input.size(); out < end; ++out)
		for(auto &out: m_frag_input)
		{
			out = (*i0 * m_inv_z0 * w1 + *i1 * m_inv_z1 * w2 + *i2 * m_inv_z2 * w3) * z_world;
			i0 += 1;
			i1 += 1;
			i2 += 1;
		}
		// write render target
		color4f out_color;
		if (!m_material->fragment_shader(m_frag_input.data(), out_color))
			return;
		// write fragment buffer
		out_color.r *= out_color.a;
		out_color.g *= out_color.a;
		out_color.b *= out_color.a;
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, out_color);
	}

	void CTile::operator()(int x, int y, const vec4f & z, const vec4i &is_inside,
		const vec4f & w1, const vec4f & w2, const vec4f & w3)
	{
		// interpolate vertex attributes
		if(m_correction)
			_interp_with_correction(w1, w2, w3);
		else
			_interp(w1, w2, w3);
		// write frame buffer
		x += center.x;
		y = m_transform_y - y;
		vec2i screen_pos[4] = {
			{ x, y },{ x + 1, y },
			{ x, y - 1 },{ x + 1, y - 1 },
		};
		auto &depth = m_rt->get_depth_buffer();
		auto &surf = m_rt->get_color_buffer();
		for (int i = 0; i < 4; ++i) {
			if (is_inside[i] >= 0) {
				// is inside 
				auto &pos = screen_pos[i];
				auto d = *depth.get<float>(pos.x, pos.y);
				if (z[i] >= d)
					continue;
				depth.set(pos.x, pos.y, z[i]);
				// write render target
				color4f out_color;
				// #3 fragment shader
				if (!m_material->fragment_shader(m_frag_interp[i], out_color, &m_ctx))
					continue;
				// write fragment buffer
				out_color.r *= out_color.a;
				out_color.g *= out_color.a;
				out_color.b *= out_color.a;
				surf.set(pos.x, pos.y, out_color);
			}
		}
	}

	void CTile::_interp(const vec4f & w1, const vec4f & w2, const vec4f & w3)
	{
		const float *i0 = m_v0, *i1 = m_v1, *i2 = m_v2;
		float *out = &m_frag_input[0];
		for (int j = 0; j < 4; ++j) {
			for (unsigned i = 0; i < m_stride; ++i, ++out)
			{
				*out = i0[i] * w1[j] + i1[i] * w2[j] + i2[i] * w3[j];
			}
		}
	}

	void CTile::_interp_with_correction(const vec4f & w1, const vec4f & w2, const vec4f & w3)
	{
		vec4f z_world;
		z_world = w1 * m_inv_z0 + w2 * m_inv_z1 + w3 * m_inv_z2;
		z_world.invert();
		const float *i0 = m_v0, *i1 = m_v1, *i2 = m_v2;
		float *out = &m_frag_input[0];
		for (int j = 0; j < 4; ++j) {
			for (unsigned i = 0; i < m_stride; ++i, ++out)
			{
				*out = (i0[i] * m_inv_z0 * w1[j] + i1[i] * m_inv_z1 * w2[j] + i2[i] * m_inv_z2 * w3[j]) * z_world[j];
			}
		}
	}

	void CTile::clear(const color4f &c)
	{
		box2i b = bounding;
		b.min += center;
		b.max += center;
		int h = m_rt->height();
		//color4f bg(0, 0, 0, 1.f);
		auto &surf = m_rt->get_color_buffer();
		for (auto y = b.min.y; y < b.max.y; ++y) {
			for (auto x = b.min.x; x < b.max.x; ++x) {
				auto ty = h - y - 1;
				//assert(bg == *surf.get<color4f>(x, ty));
				surf.set(x, ty, c);
			}
		}
	}

} // namespace wyc