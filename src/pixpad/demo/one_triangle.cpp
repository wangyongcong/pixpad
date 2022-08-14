#include <vector>
#include <stack>
#include <algorithm>
#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_tile_buffer.h"
#include "spw_rasterizer.h"
#include "imgui.h"
#include "stb/stb_log.h"
#include "metric.h"

using namespace wyc;

struct ContextOneTriangle
{
	// frame buffer
	uint32_t *color_buf;
	unsigned width, height;
	BlockArena *arena;
	RenderTarget rt;

	vec2f triangle[3];
	TileQueue full_tiles, partial_tiles;
	std::vector<vec2i> lod_full[SPW_LOD_COUNT];
	std::vector<vec2i> lod_partial[SPW_LOD_COUNT];
	std::vector<vec2i> pixels;
	
	ContextOneTriangle()
	: color_buf(nullptr)
	, width(0)
	, height(0)
	, arena(nullptr)
	, full_tiles()
	, partial_tiles()
	{
		
	}
	
	~ContextOneTriangle()
	{
		if(arena)
		{
			delete arena;
			arena = nullptr;
		}
		if(color_buf)
		{
			delete [] color_buf;
			color_buf = nullptr;
		}
	}
};

static ContextOneTriangle* get_context()
{
	static ContextOneTriangle s_context;
	return &s_context;
}

vec2i get_location(long offset, int block_per_row)
{
	int i = offset & 0xF;
	int y = i / 4;
	int x = i % 4;
	offset >>= 4;
	i = offset & 0xF;
	y += (i / 4) * 4;
	x += (i % 4) * 4;
	offset >>= 4;
	i = offset & 0xF;
	y += (i / 4) * 16;
	x += (i % 4) * 16;
	offset >>= 4;
	y += int(offset / block_per_row) * 64;
	x += int(offset % block_per_row) * 64;
	return {x, y};
}

void save_tile_coordinate(const TileQueue &tiles, bool is_partial)
{
	auto context = get_context();
	const RenderTarget &rt = context->rt;
	const int block_per_row = rt.pitch / (SPW_BLOCK_SIZE * rt.pixel_size);
	std::vector<vec2i> *tile_array;
	if(is_partial)
		tile_array = context->lod_partial;
	else
		tile_array = context->lod_full;
	for(auto *b = tiles.head; b; b = b->_next)
	{
		assert(b->lod < SPW_LOD_COUNT);
		long offset = long(b->storage - rt.storage);
		tile_array[b->lod].emplace_back(get_location(offset / rt.pixel_size, block_per_row));
	}
}

void scan_partial_queue(const Triangle *prim, TileQueue *queue, BlockArena *arena, TileQueue *full_tiles, TileQueue *partial_tiles) {
	TileQueue output;
	while(queue->head) {
		TileBlock *t = queue->pop();
		scan_tile(prim, t, arena, full_tiles, &output);
		arena->free(t);
		if(output.head) {
			save_tile_coordinate(output, true);
			if(output.head->lod < SPW_LOD_MAX)
				scan_partial_queue(prim, &output, arena, full_tiles, partial_tiles);
			else
				partial_tiles->join(&output);
		}
	}
}

void demo_one_triangle()
{
	log_info("draw one triangle");

	auto context = get_context();

	auto *verts = context->triangle;
	verts[0] = {60.9f, 250.1f};
	verts[1] = {240.5f, 40.8f};
	verts[2] = {480.6f, 370.3f};

	constexpr unsigned CANVAS_WIDTH = 2048, CANVAS_HEIGHT = 2048;
	unsigned size = CANVAS_WIDTH * CANVAS_HEIGHT;
	context->color_buf = new uint32_t[size];
	memset(context->color_buf, 0, sizeof(uint32_t) * size);
	context->width = CANVAS_WIDTH;
	context->height = CANVAS_HEIGHT;
	
	context->arena = new BlockArena(1024);
	
	RenderTarget &rt = context->rt;
	rt.storage = (char*)context->color_buf;
	rt.pixel_size = sizeof(uint32_t);
	rt.pitch = context->width * rt.pixel_size;
	rt.w = context->width;
	rt.h = context->height;
	rt.x = 0;
	rt.y = 0;
	
	Triangle prim;
	setup_triangle(&prim, verts, verts + 1, verts + 2);
	
	TileQueue partial_blocks;
	
	{		
		TIMER(draw);
		scan_block(&rt, &prim, context->arena, &context->full_tiles, &partial_blocks);
		save_tile_coordinate(partial_blocks, true);
		scan_partial_queue(&prim, &partial_blocks, context->arena, &context->full_tiles, &context->partial_tiles);
		for(auto it = context->partial_tiles.head; it; it = it->_next)
		{
			draw_tile(&prim, it, [](char *dst, const vec3f &w) {
				auto context = get_context();
				const RenderTarget &rt = context->rt;
				const int block_per_row = rt.pitch / (SPW_BLOCK_SIZE * rt.pixel_size);
				long offset = long(dst - rt.storage);
				context->pixels.emplace_back(get_location(offset / rt.pixel_size, block_per_row));
			});
		}
	}
	
	save_tile_coordinate(context->full_tiles, false);
	size_t count_full = 0, count_partial = 0;
	log_info("     FULL  PART");
	for(int i = 0; i < SPW_LOD_COUNT; ++i) {
		count_full += context->lod_full[i].size();
		count_partial += context->lod_partial[i].size();
		log_info("[%d] %5d %5d", i, context->lod_full[i].size(), context->lod_partial[i].size());
	}
	log_info("SUM %5d %5d", (unsigned)count_full, (unsigned)count_partial);
}

void transform(ImVec2 *vertices, unsigned count, const Imath::V2f &scale, const Imath::V2f &translate)
{
	Imath::V2f *vec = (Imath::V2f*)vertices;
	for(unsigned i = 0; i < count; ++i) {
		vec[i] *= scale;
		vec[i] += translate;
	}
}

void draw_block(ImVec2 *verts, const vec2f &b, int size, const Imath::V2f &scale, const Imath::V2f &translate, ImU32 color)
{
	float x = b.x;
	float y = b.y;
	float z = x + size;
	float w = y + size;
	verts[0] = {x, y};
	verts[1] = {z, y};
	verts[2] = {z, w};
	verts[3] = {x, w};
	transform(verts, 4, scale, translate);
	ImGui::GetOverlayDrawList()->AddQuad(verts[0], verts[1], verts[2], verts[3], color);
}

void fill_block(ImVec2 *verts, const vec2f &b, int size, const Imath::V2f &scale, const Imath::V2f &translate, ImU32 color)
{
	float x = b.x;
	float y = b.y;
	float z = x + size;
	float w = y + size;
	verts[0] = {x, y};
	verts[1] = {z, y};
	verts[2] = {z, w};
	verts[3] = {x, w};
	transform(verts, 4, scale, translate);
	ImGui::GetOverlayDrawList()->AddQuadFilled(verts[0], verts[1], verts[2], verts[3], color);
}

void draw_one_triangle()
{
	// detail level
	int show_lod = 2;
	assert(show_lod <= SPW_LOD_MAX);
	// camera zoom
	float zoom = 4.0f;
	// focus view
	bool is_focus = true;

	ImGuiIO &io = ImGui::GetIO();
	auto display_size = io.DisplaySize;
	
	auto context = get_context();
	auto *triangles = context->triangle;
	
	// canvas transform
	vec2f canvas_size(float(context->width), float(context->height));
	vec2f canvas_center = canvas_size * 0.5f;
	constexpr float margin = 10;
	float ratio = std::min((display_size.x - margin) / canvas_size.x, (display_size.y - margin) / canvas_size.y);
	Imath::V2f canvas_translate = {
		(display_size.x - canvas_size.x * ratio) * 0.5f,
		(display_size.y + canvas_size.y * ratio) * 0.5f
	};
	Imath::V2f canvas_scale = {
		ratio, -ratio
	};
	
	// world transform
	vec2f translate;
	vec2f scale;
	if(is_focus)
	{
		translate.setValue(0, 0);
		scale.setValue(zoom, zoom);
		for(int i = 0; i < 3; ++i) {
			translate.x += triangles[i].x;
			translate.y += triangles[i].y;
		}
		translate /= -3.2f;
		// multiply view transform
		translate *= zoom;
		translate += canvas_center;
		// multiply canvas transform
		scale *= canvas_scale;
		translate *= canvas_scale;
		translate += canvas_translate;
	}
	else {
		translate = canvas_translate;
		scale = canvas_scale;
	}

	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->PushClipRectFullScreen();
	
	// draw canvas
	std::vector<ImVec2> verts;
	verts.resize(4);
	verts[0] = {0, 0};
	verts[1] = {canvas_size.x, 0};
	verts[2] = {canvas_size.x, canvas_size.y};
	verts[3] = {0, canvas_size.y};
	transform(verts.data(), 4, canvas_scale, canvas_translate);
	draw_list->AddQuad(verts[0], verts[1], verts[2], verts[3], 0xFFFFFFFF);
	
	// draw tiles
	int block_size = SPW_BLOCK_SIZE >> (show_lod * 2);
/*	for(auto &b: context->lod_partial[show_lod])
	{
		draw_block(verts.data(), b, block_size, scale, translate, 0xFF00FFFF);
	} */
	for(int lod = show_lod; lod >= 0; --lod)
	{
		block_size = SPW_BLOCK_SIZE >> (lod * 2);
		for(auto &b : context->lod_full[lod])
		{
			fill_block(verts.data(), b, block_size, scale, translate, 0x8000FF00);
		}
	}
	// draw pixels
	for(auto &b : context->pixels)
	{
		fill_block(verts.data(), b, 1, scale, translate, 0x8000FF00);
	}

	// draw triangle
//	verts[0] = {triangles[0].x, triangles[0].y};
//	verts[1] = {triangles[1].x, triangles[1].y};
//	verts[2] = {triangles[2].x, triangles[2].y};
//	transform(verts.data(), 3, scale, translate);
//	draw_list->AddTriangle(verts[0], verts[1], verts[2], 0xFF0000FF);
	
	draw_list->PopClipRect();
}
