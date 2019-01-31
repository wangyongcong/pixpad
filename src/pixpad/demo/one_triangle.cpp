#include "spw_tile_buffer.h"
#include "ImathVec.h"
#include "vecmath.h"
#include "imgui_pixpad.h"
#include "imgui.h"

struct ContextOneTriangle
{
	wyc::CSpwTileBuffer<uint32_t> buf;
	wyc::vec2f triangle[3];
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
	
	auto *triangle = context->triangle;
	triangle[0] = {34.5, 17.8};
	triangle[1] = {14.9, 25.1};
	triangle[2] = {48.6, 37.3};
}

void draw_one_triangle()
{
	auto context = get_demo_context();
	show_pixpad(context->buf, context->triangle, 3);
}
