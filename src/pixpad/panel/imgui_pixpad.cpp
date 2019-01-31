#include "imgui.h"
#include "imgui_pixpad.h"


void show_pixpad(const wyc::CSpwTileBuffer<uint32_t> &buf, const wyc::vec2f *triangles, unsigned vertex_count)
{
	constexpr int cell_size = 11.2;
	constexpr int point_size = 2;
	constexpr int origin_x = 281.6, origin_y = 3.2;
	
	int row = (int)buf.width(), col = (int)buf.height();
	if(row < 1 || col < 1)
		return;
	
	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->PushClipRectFullScreen();
	
	ImVec2 x0, x1;
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x + col * cell_size;
	x1.y = origin_y;
	for(int r = 0; r <= row; ++r)
	{
		draw_list->AddLine(x0, x1, 0xFF00FF00);
		x0.y += cell_size;
		x1.y += cell_size;
	}
	
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x;
	x1.y = origin_y + row * cell_size;
	for(int c = 0; c <= col; ++c)
	{
		draw_list->AddLine(x0, x1, 0xFF00FF00);
		x0.x += cell_size;
		x1.x += cell_size;
	}
	
	ImVec2 va, vb, vc, vd;
	x0.x = origin_x + cell_size * 0.5 - point_size * 0.5;
	va.y = origin_y + cell_size * 0.5 - point_size * 0.5;
	for(int r = 0; r < row; ++r, x0.y += cell_size)
	{
		va.x = x0.x;
		vb.x = va.x + point_size; vb.y = va.y;
		vc.x = vb.x; vc.y = vb.y + point_size;
		vd.x = va.x; vd.y = vc.y;
		for(int c = 0; c < col; ++c)
		{
			draw_list->AddQuadFilled(va, vb, vc, vd, buf.get(c, r));
			va.x += cell_size;
			vb.x += cell_size;
			vc.x += cell_size;
			vd.x += cell_size;
		}
		va.y += cell_size;
	}
	
	auto *end = triangles + ((vertex_count / 3) * 3);
	for(auto *triangle = triangles; triangle != end; triangle += 3)
	{
		draw_list->AddTriangle(
			{origin_x + cell_size * triangle[0].x, origin_y + cell_size * triangle[0].y},
			{origin_x + cell_size * triangle[1].x, origin_y + cell_size * triangle[1].y},
			{origin_x + cell_size * triangle[2].x, origin_y + cell_size * triangle[2].y},
			0xff000000, 1.0f
		);
	}
	
	draw_list->PopClipRect();
}

