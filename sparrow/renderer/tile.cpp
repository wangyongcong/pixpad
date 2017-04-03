#include "tile.h"

namespace wyc
{
	CTile::CTile(CSpwRenderTarget *rt, Imath::Box2i &b, Imath::V2i &c)
		: bounding(b)
		, center(c)
		, m_rt(rt)
		, m_material(nullptr)
	{
	}

	void CTile::set_triangle(const float* v0, const float* v1, const float* v2, unsigned vertex_stride, const CMaterial *material)
	{
		m_v0 = v0;
		m_v1 = v1;
		m_v2 = v2;
		if (vertex_stride != m_fragment_input.size())
		{
			m_fragment_input.resize(vertex_stride, 0);
		}
		m_material = material;
	}

	void CTile::operator() (int x, int y) {

	}

	void CTile::operator() (int x, int y, float z, float w1, float w2, float w3) {
		// todo: z-test first

		// interpolate vertex attributes
		const float *i0 = m_v0, *i1 = m_v1, *i2 = m_v2;
		//for(float *out = m_fragment_input.data(), *end = out + m_fragment_input.size(); out < end; ++out)
		for(auto &out: m_fragment_input)
		{
			out = *i0 * w1 + *i1 * w2 + *i2 * w3;
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
		x += center.x;
		y = m_rt->height() - (y + center.y) - 1;
		auto &surf = m_rt->get_color_buffer();
		//unsigned v2 = *surf.get<unsigned>(x, y);
		//assert(v2 == 0xff000000);
		surf.set(x, y, v);
	}

	void CTile::clear(const Imath::C3f &c)
	{
		Imath::Box2i b = bounding;
		b.min += center;
		b.max += center;
		int h = m_rt->height();
		unsigned v = Imath::rgb2packed(c);
		unsigned bg = Imath::rgb2packed(Color3f{ 0, 0, 0 });
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