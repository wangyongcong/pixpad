#include "tile.h"

namespace wyc
{
	CTile::CTile(CSpwRenderTarget *rt, Imath::Box2i &b, Imath::V2i &c)
		: bounding(b)
		, center(c)
		, m_rt(rt)
		, m_material(nullptr)
		, m_stride(0)
	{
		m_transform_y = m_rt->height() - center.y - 1;
	}

	void CTile::set_fragment(unsigned vertex_stride, const CMaterial *material) {
		auto buff_size = vertex_stride * 4;
		if (buff_size != m_fragment_input.size())
			m_fragment_input.resize(buff_size, 0);
		m_material = material;
		m_stride = vertex_stride;
	}

	void CTile::operator() (int x, int y) {
		Imath::C4f out_color;
		if (!m_material->fragment_shader(m_fragment_input.data(), out_color))
			return;
		out_color.r *= out_color.a;
		out_color.g *= out_color.a;
		out_color.b *= out_color.a;
		y = m_rt->height() - y - 1;
		unsigned v = Imath::rgb2packed(out_color);
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, v);
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
		for(auto &out: m_fragment_input)
		{
			out = (*i0 * m_inv_z0 * w1 + *i1 * m_inv_z1 * w2 + *i2 * m_inv_z2 * w3) * z_world;
			i0 += 1;
			i1 += 1;
			i2 += 1;
		}
		// write render target
		Imath::C4f out_color;
		if (!m_material->fragment_shader(m_fragment_input.data(), out_color))
			return;
		// write fragment buffer
		out_color.r *= out_color.a;
		out_color.g *= out_color.a;
		out_color.b *= out_color.a;
		unsigned v = Imath::rgb2packed(out_color);
		auto &surf = m_rt->get_color_buffer();
		//unsigned v2 = *surf.get<unsigned>(x, y);
		//assert(v2 == 0xff000000);
		surf.set(x, y, v);
	}

	void CTile::operator()(int x, int y, const Imath::V4f & z, const Imath::V4i &is_inside,
		const Imath::V4f & w1, const Imath::V4f & w2, const Imath::V4f & w3)
	{
		x += center.x;
		y = m_transform_y - y;
		Imath::V2i screen_pos[4] = {
			{x, y}, {x + 1, y},
			{x, y - 1}, {x + 1, y - 1},
		};
		// interpolate vertex attributes
		const float *i0 = m_v0, *i1 = m_v1, *i2 = m_v2;
		Imath::V4f z_world;
		z_world = w1 * m_inv_z0 + w2 * m_inv_z1 + w3 * m_inv_z2;
		z_world.x = 1.0f / z_world.x;
		z_world.y = 1.0f / z_world.y;
		z_world.z = 1.0f / z_world.z;
		z_world.w = 1.0f / z_world.w;
		float *out[4] = { &m_fragment_input[0], &m_fragment_input[m_stride], 
			&m_fragment_input[m_stride*2], &m_fragment_input[m_stride*3] };
		for (unsigned i = 0; i < m_stride; ++i) {
			*out[0] = (*i0 * m_inv_z0 * w1[0] + *i1 * m_inv_z1 * w2[0] + *i2 * m_inv_z2 * w3[0]) * z_world[0];
			*out[1] = (*i0 * m_inv_z0 * w1[1] + *i1 * m_inv_z1 * w2[1] + *i2 * m_inv_z2 * w3[1]) * z_world[1];
			*out[2] = (*i0 * m_inv_z0 * w1[2] + *i1 * m_inv_z1 * w2[2] + *i2 * m_inv_z2 * w3[2]) * z_world[2];
			*out[3] = (*i0 * m_inv_z0 * w1[3] + *i1 * m_inv_z1 * w2[3] + *i2 * m_inv_z2 * w3[3]) * z_world[3];
			++out[0];
			++out[1];
			++out[2];
			++out[3];
			i0 += 1;
			i1 += 1;
			i2 += 1;
		}
		// write frame buffer
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
				Imath::C4f out_color;
				if (!m_material->fragment_shader(out[i], out_color))
					continue;
				// write fragment buffer
				out_color.r *= out_color.a;
				out_color.g *= out_color.a;
				out_color.b *= out_color.a;
				unsigned v = Imath::rgb2packed(out_color);
				//unsigned v2 = *surf.get<unsigned>(x, y);
				//assert(v2 == 0xff000000);
				surf.set(pos.x, pos.y, v);
			}
		}
	}

	void CTile::clear(const Imath::C3f &c)
	{
		Imath::Box2i b = bounding;
		b.min += center;
		b.max += center;
		int h = m_rt->height();
		unsigned v = Imath::rgb2packed(c);
		unsigned bg = Imath::rgb2packed(Imath::C3f{ 0, 0, 0 });
		auto &surf = m_rt->get_color_buffer();
		for (auto y = b.min.y; y < b.max.y; ++y) {
			for (auto x = b.min.x; x < b.max.x; ++x) {
				auto ty = h - y - 1;
				//assert(bg == *surf.get<unsigned>(x, ty));
				surf.set(x, ty, v);
			}
		}
	}

} // namespace wyc