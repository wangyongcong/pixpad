#pragma once
#include "test.h"
#include <cmath>
#include "image.h"
#include "spw_renderer.h"
#include "clipping.h"
#include "spw_rasterizer.h"

class CSimplePlotter
{
public:
	CSimplePlotter(wyc::CSpwRenderTarget *rt, Imath::C4f frag_color)
		: m_rt(rt)
	{
		frag_color.r *= frag_color.a;
		frag_color.g *= frag_color.a;
		frag_color.b *= frag_color.a;
		m_color = frag_color;
	}

	void operator() (int x, int y)
	{
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, m_color);
	}

protected:
	wyc::CSpwRenderTarget *m_rt;
	Imath::C4f m_color;
};

class CTestLine : public CTest
{
public:
	virtual void run()
	{
		// clear frame buffer
		m_renderer->process();

		// direct write to render target
		auto render_target = std::dynamic_pointer_cast<wyc::CSpwRenderTarget>(m_renderer->get_render_target());
		CSimplePlotter plotter(render_target.get(), { 0, 1.0f, 0, 1.0f });

		Imath::V2f center = { m_image_w * 0.5f, m_image_h * 0.5f };
		Imath::V2f beg, end;
		Imath::Box2f clip_window = { { 0.5f, 0.5f },{ m_image_w - 0.5f, m_image_h - 0.5f } };
		float radius = float(m_image_w + m_image_h);
		double pi_180 = M_PI / 180.0f;
		for (int i = 0; i < 360; i += 5)
		{
			auto rad = i * pi_180;
			beg = center;
			end.x = float(radius * std::cos(rad)) + beg.x;
			end.y = float(radius * std::sin(rad)) + beg.y;
			if (wyc::clip_line(beg, end, clip_window))
			{
				wyc::draw_line(plotter, beg, end);
			}
		}
		save_image("line.png");
	}
};

REGISTER_TEST(CTestLine)
