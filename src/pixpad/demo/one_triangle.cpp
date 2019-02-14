#include <vector>
#include "ImathVec.h"
#include "vecmath.h"
#include "spw_tile_buffer.h"
#include "spw_rasterizer.h"
#include "imgui.h"
#include "stb_log.h"

using namespace wyc;

#define SUB_PIXEL_PRECISION 8

struct TileState {
	bool trival_reject;
	uint8_t trival_accept = 0;
};

struct ContextOneTriangle
{
	CSpwTileBuffer<uint32_t> buf;
	vec2f triangle[3];
	std::vector<TileState> tile_states;
	vec2i vertex[3];
	vec2i edges[3];
	unsigned tile_index;
	unsigned edge_index;
};

static ContextOneTriangle* get_demo_context()
{
	static ContextOneTriangle s_context;
	return &s_context;
}

void demo_one_triangle()
{
	auto context = get_demo_context();
	auto &buf = context->buf;
	buf.storage(64, 64);
	uint32_t colors[] = {
		0xFFFFFFFF, 0xFFFF0000, 0xFF0000FF,
		0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
		0xFF88FF00, 0xFF8800FF, 0xFF00FF88,
		0xFF0088FF,
	};
	unsigned color_count = sizeof(colors) / sizeof(uint32_t);
	for(auto i=0u; i < buf.tile_count(); ++i) {
		buf.set_tile(i, colors[i % color_count]);
	}
	
	auto &triangle = context->triangle;
	triangle[0] = {6.9f, 25.1f};
	triangle[1] = {24.5f, 4.8f};
	triangle[2] = {48.6f, 37.3f};
	
	auto &verts = context->vertex;
	verts[0] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[0]);
	verts[1] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[1]);
	verts[2] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[2]);

	auto &edges = context->edges;
	edges[0] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[1] - triangle[0]);
	edges[1] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[2] - triangle[1]);
	edges[2] = snap_to_subpixel<SUB_PIXEL_PRECISION>(triangle[0] - triangle[2]);
	
	auto &tile_states = context->tile_states;
	tile_states.resize(buf.tile_count());
	for(auto &t: tile_states){
		t.trival_reject = false;
		t.trival_accept = 0;
	}
}

void next_tile()
{
	auto context = get_demo_context();
	if(context->tile_index >= context->tile_states.size() || context->edge_index >= 3) {
		return;
	}
	auto &buf = context->buf;

	const int tile_size = buf.tile_size() << SUB_PIXEL_PRECISION;
	wyc::vec2i corner[2];
	const vec2i edge = context->edges[context->edge_index];
	float t = edge.x * edge.y;
	if(t >= 0) {
		// 1/3象限
		corner[0] = {tile_size, 0};
		corner[1] = {0, tile_size};
	}
	else {
		// 2/4象限
		corner[0] = {0, 0};
		corner[1] = {tile_size, tile_size};
	}
//	if(t == 0) {
//		// 与坐标轴平行, 可以不用算edge function
//		if(e.x == 0) {
//			corner[0] = {0, 0};
//			corner[1] = {0, tile_size};
//		}
//		else {
//			corner[0] = {0, 0};
//			corner[1] = {tile_size, 0};
//		}
//	} // t == 0
	auto &tile = context->tile_states[context->tile_index];
	int row = context->tile_index / buf.tile_col();
	int col = context->tile_index % buf.tile_col();
	corner[0].x += tile_size * col;
	corner[0].y += tile_size * row;
	corner[1].x += tile_size * col;
	corner[1].y += tile_size * row;

	const auto &v0 = context->vertex[context->edge_index];
	auto e0 = corner[0] - v0;
	auto e1 = corner[1] - v0;
	auto t0 = wyc::edge_function_fixed(edge, e0);
	auto t1 = wyc::edge_function_fixed(edge, e1);
	// most positive
	auto w = std::max(t0, t1);
	if(w < 0) {
		tile.trival_reject = true;
	}
	else {
		tile.trival_accept += 1;
	}
	
	// advace to next tile/edge
	context->tile_index += 1;
	if(context->tile_index >= context->tile_states.size())
	{
		context->tile_index = 0;
		context->edge_index += 1;
	}
}

void draw_one_triangle()
{
	auto context = get_demo_context();
	auto &buf = context->buf;
	auto &triangles = context->triangle;
	auto &tile_states = context->tile_states;

	constexpr float cell_size = 11.2f;
	constexpr float cell_thickness = 1.0f;
	constexpr float point_size = 2.0f;
	constexpr float canvas_width = 1280.0f, canvas_height = 720.0f;
	
	const int row = (int)buf.width(), col = (int)buf.height();
	if(row < 1 || col < 1)
		return;
	
	const float origin_x = (canvas_width - cell_size * col - cell_thickness) * 0.5f;
	const float origin_y = (canvas_height - cell_size * row - cell_thickness) * 0.5f;
	
	if(ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Space)))
	{
		next_tile();
		log_info("tile[%d]", context->tile_index);
	}
	
	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->PushClipRectFullScreen();
	
	ImVec2 x0, x1;
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x + col * cell_size;
	x1.y = origin_y;
	for(int r = 0; r <= row; ++r)
	{
		draw_list->AddLine(x0, x1, 0xFFFFFFFF, cell_thickness);
		x0.y += cell_size;
		x1.y += cell_size;
	}
	
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x;
	x1.y = origin_y + row * cell_size;
	for(int c = 0; c <= col; ++c)
	{
		draw_list->AddLine(x0, x1, 0xFFFFFFFF, cell_thickness);
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
			draw_list->AddQuadFilled(va, vb, vc, vd, 0xFFFFFFFF);
			va.x += cell_size;
			vb.x += cell_size;
			vc.x += cell_size;
			vd.x += cell_size;
		}
		va.y += cell_size;
	}
	
	float top = origin_y + cell_size * row;
	ImColor color;
	const float tile_size = buf.tile_size() * cell_size;
	int tile_index = 0;
	va.y = top - tile_size;
	for(int r = 0; r < buf.tile_row(); ++r)
	{
		va.x = origin_x;
		vb.x = va.x + tile_size; vb.y = va.y;
		vc.x = vb.x; vc.y = vb.y + tile_size;
		vd.x = va.x; vd.y = vc.y;
		for(int c = 0; c < buf.tile_col(); ++c)
		{
			assert(tile_index < tile_states.size());
			auto &t = tile_states[tile_index++];
			if(t.trival_reject)
				color = 0x800000FF;
			else if(t.trival_accept >= 3)
				color = 0x8000FF00;
			else
				color = 0x80FFFFFF;
			draw_list->AddQuadFilled(va, vb, vc, vd, color);
			va.x += tile_size;
			vb.x += tile_size;
			vc.x += tile_size;
			vd.x += tile_size;
		}
		va.y -= tile_size;
	}
	
	ImVec2 tri[3] = {
		{origin_x + cell_size * triangles[0].x, top - cell_size * triangles[0].y},
		{origin_x + cell_size * triangles[1].x, top - cell_size * triangles[1].y},
		{origin_x + cell_size * triangles[2].x, top - cell_size * triangles[2].y},
	};

	ImColor edge_color1 = 0xFF00FF00;
	ImColor edge_color2 = 0xFFFF0000;
	draw_list->AddLine(tri[0], tri[1], context->edge_index == 0 ? edge_color2 : edge_color1);
	draw_list->AddLine(tri[1], tri[2], context->edge_index == 1 ? edge_color2 : edge_color1);
	draw_list->AddLine(tri[2], tri[0], context->edge_index == 2 ? edge_color2 : edge_color1);
	
	draw_list->PopClipRect();
}
