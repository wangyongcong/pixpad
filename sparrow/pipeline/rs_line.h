#pragma once

namespace wyc
{

// line rasterization algorithms

template<typename PLOTTER>
void line_bresenham(PLOTTER &plot, const vec2f &v0, const vec2f &v1)
{
	int x0, y0, x1, y1;
	x0 = fast_round(v0.x);
	y0 = fast_round(v0.y);
	x1 = fast_round(v1.x);
	y1 = fast_round(v1.y);
	if (x0>x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int advance;
	if (dy < 0)
	{
		dy = -dy;
		advance = -1;
	}
	else
		advance = 1;
	plot(x0, y0);
	if (dx >= dy)
	{
		int m = dy << 1;
		int d = -dx;
		for (int i = 0; i<dx; ++i)
		{
			plot += 1;
			d += m;
			if (d >= 0)
			{
				plot.skip_row(advance);
				d -= dx << 1;
			}
			plot();
		}
	}
	else
	{
		int m = dx << 1;
		int d = -dy;
		for (int i = 0; i < dy; ++i)
		{
			plot.skip_row(advance);
			d += m;
			if (d >= 0)
			{
				plot += 1;
				d -= dy << 1;
			}
			plot();
		}
	}
}

} // namespace wyc