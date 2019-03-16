#include <vector>
#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_tile_buffer.h"
#include "spw_rasterizer.h"
#include "imgui.h"
#include "stb_log.h"

using namespace wyc;

#define SUB_PIXEL_PRECISION 8
#define HALF_PIXEL_PRECISION 4

struct TileState {
	bool trivial_reject = false;
	bool trivial_accept = false;
};

struct ContextOneTriangle
{
	CSpwTileBuffer<uint32_t> buf;
	vec2f triangle[3];
	vec2i vertex[3];
	vec2i edges[3];
	unsigned tile_index;
	unsigned edge_index;
	std::vector<TileState> tile0;
	std::vector<TileState> tile1;
	std::vector<TileState> tile2;
	int show_tile_index;
};

static ContextOneTriangle* get_demo_context()
{
	static ContextOneTriangle s_context;
	return &s_context;
}

void fill_tiles(void);

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
	
	context->show_tile_index = -1;
	fill_tiles();
}

void fill_tiles()
{
	constexpr int canvas_size = 64;
	constexpr int tile_size = 16;
	constexpr int tile_size_shift = 4;
	constexpr int tile_size_hp = 16 << SUB_PIXEL_PRECISION;
	constexpr int tile_row = canvas_size / tile_size;
	constexpr int tile_col = tile_row;
	constexpr int tile_count = tile_row * tile_col;

	struct Block {
		int row;
		int col;
		int reject[3];
		int accept[3];
		
		Block() {
			reject[0] = reject[1] = reject[2] = 0;
			accept[0] = accept[1] = accept[2] = 0;
		}
	};
	
	auto context = get_demo_context();
	const vec2i *edges = context->edges;
	const vec2i *vertex = context->vertex;
	const vec2i &v0 = vertex[0];
	const vec2i &v1 = vertex[1];
	const vec2i &v2 = vertex[2];
	
	// vertex index of edges
	const vec2i edge_index[3] = {
		{0, 1},
		{1, 2},
		{2, 0},
	};
	// edge function step factor
	const vec2i edge_step[3] = {
		{v0.y - v1.y, v1.x - v0.x},
		{v1.y - v2.y, v2.x - v1.x},
		{v2.y - v0.y, v0.x - v2.x},
	};
	// high precision edge function value at reject corner
	int64_t edge_reject_hp[3];
	// reject value offset of 4x4 sub-blocks
	mat4i edge_mat[3];
	int edge_ap[3];
	
	Block block_list[tile_count];
	Block* partial_blocks[tile_count];
	int partial_count = 0;
	
	// edges are in counter-clockwise
	for(auto e = 0; e < 3; ++e)
	{
		const vec2i &edge = edges[e];
		const vec2i &step = edge_step[e];
		const vec2i &index = edge_index[e];
		vec2i rp;  // reject point
		vec2i ap;  // accpet point
		if(edge.x >= 0) {
			if(edge.y >= 0) {
				// point to 1st quadrant
				// the trivial reject point is at *
				// * -- o
				// |    |
				// o -- o
				rp.setValue(0, tile_size_hp);
				ap.setValue(step.x, -step.y);
				int r1 = ap.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ap.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge_mat[e] = {
					r3, r3 + c1, r3 + c2, r3 + c3,
					r2, r2 + c1, r2 + c2, r2 + c3,
					r1, r1 + c1, r1 + c2, r1 + c3,
					0, c1, c2, c3,
				};
			}
			else {
				// point to 4th quadrant
				// the trivial reject point is at *
				// o -- *
				// |    |
				// o -- o
				rp.setValue(tile_size_hp, tile_size_hp);
				ap.setValue(-step.x, -step.y);
				int r1 = ap.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ap.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge_mat[e] = {
					r3 + c3, r3 + c2, r3 + c1, r3,
					r2 + c3, r2 + c2, r2 + c1, r2,
					r1 + c3, r1 + c2, r1 + c1, r1,
					c3, c2, c1, 0,
				};
			}
		}
		else {
			if(edge.y >= 0) {
				// point to 2nd quadrand
				// the trivial reject point is at *
				// o -- o
				// |    |
				// * -- o
				rp.setValue(0, 0);
				ap.setValue(step.x, step.y);
				int r1 = ap.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ap.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge_mat[e] = {
					0, c1, c2, c3,
					r1, r1 + c1, r1 + c2, r1 + c3,
					r2, r2 + c1, r2 + c2, r2 + c3,
					r3, r3 + c1, r3 + c2, r3 + c3,
				};
			}
			else {
				// point to 3rd quadrand
				// the trivial reject point is at *
				// o -- o
				// |    |
				// o -- *
				rp.setValue(tile_size_hp, 0);
				ap.setValue(-step.x, step.y);
				int r1 = ap.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ap.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge_mat[e] = {
					c3, c2, c1, 0,
					r1 + c3, r1 + c2, r1 + c1, r1,
					r2 + c3, r2 + c2, r2 + c1, r2,
					r3 + c3, r3 + c2, r3 + c1, r3,
				};
			}
		} // detect edge direction
		int64_t hp = edge_function_fixed(vertex[index.x], vertex[index.y], rp);
		edge_reject_hp[e] = hp;
		int y = int(hp >> (SUB_PIXEL_PRECISION + tile_size_shift));
		int offset = ap.x + ap.y;
		edge_ap[e] = offset;
		Block *block = block_list;
		for(auto r = 0; r < tile_row; ++r, y += step.y)
		{
			for(auto c = 0, x = y; c < tile_col; ++c, x += step.x)
			{
				block->reject[e] = x;
				block->accept[e] = x + offset;
				++block;
			}
		} // tile loop
	} // edge loop
	
	Block *block = block_list;
	auto &tile_list = context->tile0;
	tile_list.resize(tile_count, {false, false});
	auto *tile = &tile_list[0];
	for(auto r = 0; r < tile_row; ++r)
	{
		for(auto c = 0; c < tile_col; ++c, ++block, ++tile)
		{
			block->row = r;
			block->col = c;
			if((block->reject[0] | block->reject[1] | block->reject[2]) < 0) {
				// trivial reject
				tile->trivial_reject = true;
				continue;
			}
			if((block->accept[0] | block->accept[1] | block->accept[2]) > 0) {
				// trivial accept
				tile->trivial_accept = true;
			}
			else {
				// partial accept
				partial_blocks[partial_count++] = block;
			}
		}
	} // block loop
	
	// step into partial blocks
	constexpr int tile1_size = 4;
	constexpr int tile1_size_shift = 2;
	constexpr int tile1_count = tile_count * 16;
	
	context->tile1.resize(tile1_count, {true, false});
	int tails[3] = {
		int(edge_reject_hp[0] >> (SUB_PIXEL_PRECISION + tile1_size_shift)) & 3,
		int(edge_reject_hp[1] >> (SUB_PIXEL_PRECISION + tile1_size_shift)) & 3,
		int(edge_reject_hp[2] >> (SUB_PIXEL_PRECISION + tile1_size_shift)) & 3,
	};
	Block partial_tiles[partial_count * 16];
	int partial_tile_count = 0;
	for(int i = 0; i < partial_count; ++i) {
		block = partial_blocks[i];
		mat4i m0((block->reject[0] << 2) + tails[0]);
		mat4i m1((block->reject[1] << 2) + tails[1]);
		mat4i m2((block->reject[2] << 2) + tails[2]);
		m0 += edge_mat[0];
		m1 += edge_mat[1];
		m2 += edge_mat[2];
		mat4i reject_mask = m0;
		reject_mask |= m1;
		reject_mask |= m2;
		mat4i ap0 = m0 + edge_ap[0];
		mat4i ap1 = m1 + edge_ap[1];
		mat4i ap2 = m2 + edge_ap[2];
		mat4i accept_mask = ap0;
		accept_mask |= ap1;
		accept_mask |= ap2;
		int *x = (int*)reject_mask.x;
		int *y = (int*)accept_mask.x;
		int row_count = tile_col * tile1_size;
		int tile1_row = block->row * tile1_size;
		int tile1_col = block->col * tile1_size;
		int row_begin = tile1_row * row_count + tile1_col;
		for(int r = 0; r < tile1_size; ++r, row_begin += row_count)
		{
			auto *st = &context->tile1[row_begin];
			for(int c = 0; c < tile1_size; ++c, ++x, ++y, ++st)
//			for(int c = row_begin, col_end = row_begin + tile1_size; c < col_end; ++c, ++x, ++y)
			{
//				auto &st = context->tile1[c];
				if(*x < 0) {
					st->trivial_reject = true;
				}
				else {
					st->trivial_reject = false;
					if(*y > 0)
						st->trivial_accept = true;
					else {
						// partial reject
						Block &t = partial_tiles[partial_tile_count++];
						t.row = tile1_row + r;
						t.col = tile1_col + c;
						t.reject[0] = m0[r][c];
						t.reject[1] = m1[r][c];
						t.reject[2] = m2[r][c];
					}
				}
			}
		} // L1 tiles loop
	} // partial blocks loop
	
	// step into each pixel
	// top-left bias
	const int edge_bias[3] = {
		is_top_left(v0, v1) ? 0 : -1,
		is_top_left(v1, v2) ? 0 : -1,
		is_top_left(v2, v0) ? 0 : -1,
	};
	constexpr int tail_bits = HALF_PIXEL_PRECISION + 2;
	constexpr int tail_mask = (1 << tail_bits) - 1;
	auto &tile2 = context->tile2;
	tile2.resize(64 * 64, {true, false});
	for(int i = 0; i < partial_tile_count; ++i)
	{
		block = partial_tiles + i;
		// edge function at pixel center
		int64_t e0 = ((block->reject[0] << tail_bits) | ((edge_reject_hp[0] >> HALF_PIXEL_PRECISION) & tail_mask)) + edge_ap[0];
		int64_t e1 = ((block->reject[1] << tail_bits) | ((edge_reject_hp[1] >> HALF_PIXEL_PRECISION) & tail_mask)) + edge_ap[1];
		int64_t e2 = ((block->reject[2] << tail_bits) | ((edge_reject_hp[2] >> HALF_PIXEL_PRECISION) & tail_mask)) + edge_ap[2];
		mat4i m0 = int(e0 >> HALF_PIXEL_PRECISION) + edge_bias[0];
		mat4i m1 = int(e1 >> HALF_PIXEL_PRECISION) + edge_bias[1];
		mat4i m2 = int(e2 >> HALF_PIXEL_PRECISION) + edge_bias[2];
		m0 += edge_mat[0];
		m1 += edge_mat[1];
		m2 += edge_mat[2];
		mat4i reject_mask = m0;
		reject_mask |= m1;
		reject_mask |= m2;
		int *x = (int*)reject_mask.x;
		int tile2_row = block->row * 4;
		int tile2_col = block->col * 4;
		int row_count = 64;
		int row_begin = tile2_row * row_count + tile2_col;
		for(int r = 0; r < 4; ++r, row_begin += row_count)
		{
			for(int c = 0; c < 4; ++c, ++x)
			{
				if(*x >= 0) {
					auto &t = tile2[row_begin + c];
					t.trivial_reject = false;
					t.trivial_accept = true;
				}
			}
		} // pixels loop
	} // tile2 loop
}

void draw_tile(ImDrawList* draw_list, float x, float y, const TileState *tiles, int row, int col, float tile_size)
{
	ImColor color;
	ImVec2 va, vb, vc, vd;
	const TileState *tile = tiles;
	va.y = y;
	for(int r = 0; r < row; ++r)
	{
		va.x = x;
		vb.x = va.x + tile_size; vb.y = va.y;
		vc.x = vb.x; vc.y = vb.y - tile_size;
		vd.x = va.x; vd.y = vc.y;
		for(int c = 0; c < col; ++c, ++tile)
		{
			if(!tile->trivial_reject) {
				if(tile->trivial_accept)
					color = 0x8000FF00;
				else
					color = 0x8000FFFF;
				draw_list->AddQuadFilled(va, vb, vc, vd, color);
			}
			va.x += tile_size;
			vb.x += tile_size;
			vc.x += tile_size;
			vd.x += tile_size;
		}
		va.y -= tile_size;
	}
}

void draw_one_triangle()
{
	auto context = get_demo_context();
	auto &buf = context->buf;
	auto &triangles = context->triangle;

	constexpr float cell_size = 11.2f;
	constexpr float cell_thickness = 1.0f;
	constexpr float point_size = 2.0f;
	constexpr float canvas_width = 1280.0f, canvas_height = 720.0f;
	
	const int row = (int)buf.width(), col = (int)buf.height();
	if(row < 1 || col < 1)
		return;
	
	const float origin_x = (canvas_width - cell_size * col - cell_thickness) * 0.5f;
	const float origin_y = (canvas_height - cell_size * row - cell_thickness) * 0.5f;
	
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
	
//	ImVec2 va, vb, vc, vd;
//	x0.x = origin_x + cell_size * 0.5 - point_size * 0.5;
//	va.y = origin_y + cell_size * 0.5 - point_size * 0.5;
//	for(int r = 0; r < row; ++r, x0.y += cell_size)
//	{
//		va.x = x0.x;
//		vb.x = va.x + point_size; vb.y = va.y;
//		vc.x = vb.x; vc.y = vb.y + point_size;
//		vd.x = va.x; vd.y = vc.y;
//		for(int c = 0; c < col; ++c)
//		{
//			draw_list->AddQuadFilled(va, vb, vc, vd, 0xFFFFFFFF);
//			va.x += cell_size;
//			vb.x += cell_size;
//			vc.x += cell_size;
//			vd.x += cell_size;
//		}
//		va.y += cell_size;
//	}
	
	float top = origin_y + cell_size * row + cell_thickness * 0.5;
//	if(ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Space)))
	if(ImGui::IsKeyReleased('1'))
	{
		context->show_tile_index = 1;
	}
	else if(ImGui::IsKeyReleased('2'))
	{
		context->show_tile_index = 2;
	}
	else if(ImGui::IsKeyReleased('3'))
	{
		context->show_tile_index = 3;
	}
	
	if(context->show_tile_index == 1)
	{
		if (context->tile0.size() > 0)
			draw_tile(draw_list, origin_x, top, &context->tile0[0], 4, 4, 16 * cell_size);
	}
	else if(context->show_tile_index == 2)
	{
		if (context->tile1.size() > 0)
			draw_tile(draw_list, origin_x, top, &context->tile1[0], 16, 16, 4 * cell_size);
	}
	else if(context->show_tile_index == 3)
	{
		if (context->tile2.size() > 0)
			draw_tile(draw_list, origin_x, top, &context->tile2[0], 64, 64, cell_size);
	}
	
	ImVec2 tri[3] = {
		{origin_x + cell_size * triangles[0].x, top - cell_size * triangles[0].y},
		{origin_x + cell_size * triangles[1].x, top - cell_size * triangles[1].y},
		{origin_x + cell_size * triangles[2].x, top - cell_size * triangles[2].y},
	};

	draw_list->AddLine(tri[0], tri[1], 0xFFFF0000);
	draw_list->AddLine(tri[1], tri[2], 0xFFFF0000);
	draw_list->AddLine(tri[2], tri[0], 0xFFFF0000);
	
	draw_list->PopClipRect();
}
