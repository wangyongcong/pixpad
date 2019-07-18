#include <vector>
#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_tile_buffer.h"
#include "spw_rasterizer.h"
#include "imgui.h"
#include "stb_log.h"

using namespace wyc;

struct ContextOneTriangle
{
	// frame buffer
	uint32_t *color_buf;
	unsigned width, height;
	BlockArena *arena;

	vec2f triangle[3];
	TileQueue full_blocks, partial_blocks;
	TileBlock *full_tiles, *partial_tiles;
	unsigned full_block_count;
	unsigned partial_block_count;
	
	ContextOneTriangle()
	: color_buf(nullptr)
	, width(0)
	, height(0)
	, arena(nullptr)
	, full_blocks()
	, partial_blocks()
	, full_tiles(nullptr)
	, partial_tiles(nullptr)
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
	
	context->arena = new BlockArena();
	context->arena->reserve(64);
	
	RenderTarget rt;
	rt.storage = (char*)context->color_buf;
	rt.w = context->width;
	rt.h = context->height;
	rt.x = 0;
	rt.y = 0;
	
	Triangle prim;
	setup_triangle(&prim, verts, verts + 1, verts + 2);
	scan_block(&rt, &prim, context->arena, &context->full_blocks, &context->partial_blocks);
	
	if(context->full_blocks)
	{
		unsigned count = 0;
		for(TileBlock *b = context->full_blocks.head; b; b=b->_next) {
			count += 1;
		}
		context->full_block_count = count;
		log_info("full block: %d", count);
	}
	if(context->partial_blocks)
	{
		unsigned count = 0;
		for(TileBlock *b = context->partial_blocks.head; b; b=b->_next) {
			count += 1;
		}
		context->partial_block_count = count;
		log_info("partial block: %d", count);
	}
}

void transform(ImVec2 *vertices, unsigned count, const Imath::V2f &scale, const Imath::V2f &translate)
{
	Imath::V2f *vec = (Imath::V2f*)vertices;
	for(int i = 0; i < count; ++i) {
		vec[i] *= scale;
		vec[i] += translate;
	}
}

void draw_one_triangle()
{
	ImGuiIO &io = ImGui::GetIO();
	auto display_size = io.DisplaySize;
	
	auto context = get_context();
	auto &triangles = context->triangle;
	
	// draw canvas
	float canvas_width = context->width, canvas_height = context->height;
	constexpr float margin = 10;
	float ratio_x = (display_size.x - margin) / canvas_width;
	float ratio_y = (display_size.y - margin) / canvas_height;
	float ratio = std::min(ratio_x, ratio_y);
	float view_width = canvas_width * ratio, view_height = canvas_height * ratio;
	Imath::V2f translate = {
		(display_size.x - view_width) * 0.5f,
		(display_size.y + view_height) * 0.5f
	};
	Imath::V2f scale = {
		ratio, -ratio
	};

	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->PushClipRectFullScreen();
	
	std::vector<ImVec2> verts;
	verts.resize(4);
	verts[0] = {0, 0};
	verts[1] = {canvas_width, 0};
	verts[2] = {canvas_width, canvas_height};
	verts[3] = {0, canvas_height};
	transform(verts.data(), 4, scale, translate);
	draw_list->AddQuad(verts[0], verts[1], verts[2], verts[3], 0xFFFFFFFF);
	
	if(context->full_blocks)
	{
		for(TileBlock *b = context->full_blocks.head; b; b=b->_next) {
			float x = b->index.x * SPW_BLOCK_SIZE;
			float y = b->index.y * SPW_BLOCK_SIZE;
			float z = x + SPW_BLOCK_SIZE;
			float w = y + SPW_BLOCK_SIZE;
			verts[0] = {x, y};
			verts[1] = {z, y};
			verts[2] = {z, w};
			verts[3] = {x, w};
			transform(verts.data(), 4, scale, translate);
			draw_list->AddQuadFilled(verts[0], verts[1], verts[2], verts[3], 0x8000FF00);
		}
	}
	
	if(context->partial_blocks)
	{
		for(TileBlock *b = context->partial_blocks.head; b; b=b->_next) {
			float x = b->index.x * SPW_BLOCK_SIZE;
			float y = b->index.y * SPW_BLOCK_SIZE;
			float z = x + SPW_BLOCK_SIZE;
			float w = y + SPW_BLOCK_SIZE;
			verts[0] = {x, y};
			verts[1] = {z, y};
			verts[2] = {z, w};
			verts[3] = {x, w};
			transform(verts.data(), 4, scale, translate);
			draw_list->AddQuad(verts[0], verts[1], verts[2], verts[3], 0xFF00FFFF);
		}
	}
	
	verts[0] = {triangles[0].x, triangles[0].y};
	verts[1] = {triangles[1].x, triangles[1].y};
	verts[2] = {triangles[2].x, triangles[2].y};
	transform(verts.data(), 3, scale, translate);
	draw_list->AddTriangle(verts[0], verts[1], verts[2], 0xFF00FF00);
	
	draw_list->PopClipRect();
}
