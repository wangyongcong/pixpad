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
		m_color = Imath::rgb2packed(frag_color);
	}

	void operator() (int x, int y)
	{
		auto &surf = m_rt->get_color_buffer();
		surf.set(x, y, m_color);
	}

protected:
	wyc::CSpwRenderTarget *m_rt;
	unsigned m_color;
};

class CTestLine : public CTest
{
public:
	static CTest* create() {
		return new CTestLine();
	}
	virtual void run()
	{
		unsigned width = 960, height = 540;
		std::string img_file = "test_line.png";

		auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
		render_target->create(width, height, wyc::SPR_COLOR_R8G8B8A8);
		render_target->get_color_buffer().clear(0xFF000000);
		CSimplePlotter plotter(render_target.get(), { 0, 1.0f, 0, 1.0f });

		Imath::V2f center = { width * 0.5f, height * 0.5f };
		Imath::V2f beg, end;
		Imath::Box2f clip_window = { { 0.5f, 0.5f },{ width - 0.5f, height - 0.5f } };
		float radius = float(width + height);
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

		auto &buffer = render_target->get_color_buffer();
		wyc::CImage image(buffer.get_buffer(), buffer.row_length(), buffer.row(), buffer.pitch());
		if (!image.save(img_file))
		{
			log_error("Failed to save image file");
		}
	}
};