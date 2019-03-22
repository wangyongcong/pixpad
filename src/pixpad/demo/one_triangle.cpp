#include <vector>
#include "ImathVec.h"
#include "ImathMatrix.h"
#include "vecmath.h"
#include "spw_tile_buffer.h"
#include "spw_rasterizer.h"
#include "imgui.h"
#include "stb_log.h"

using namespace wyc;

// when individual coordinates are in [-2^p, 2^p-1], the result of edge function fits inside a 2*(p+2)-bit signed integer
// e.g with 32-bit integer, the precision bits [p] is 32/2-2=14, coordinates should be in range [-16384, 16383]
inline int coordinate_precision(int available_bits)
{
	return available_bits / 2 - 2;
}

inline bool is_pod(int v)
{
	return (v & (v-1)) == 0;
}

#define SUB_PIXEL_PRECISION 8
#define HALF_SUB_PIXEL_PRECISION 4
//
#define BLOCK_SIZE 16
#define BLOCK_SIZE_BITS 4
#define BLOCK_SIZE_MASK 15
//
#define TILE_SIZE 4
#define TILE_SIZE_BITS 2
#define TILE_SIZE_MASK 3

#define LOD_BITS 2
#define LOD_MASK 3


struct TriangleEdgeInfo
{
	// 4x4 reject corner offsets
	mat4i rc_steps[3];
	// offset of reject corner to accept corner
	vec3i rc2ac;
	// edge function value at reject corner (high precision)
	int64_t rc_hp[3];
	vec2i dxdy[3];
	// top-left fill rule bias
	vec3i bias;
	vec3f tail;
};

class ITileBin
{
public:
	virtual void fill_partial_block(int index, const vec3i &reject) = 0;
	virtual void fill_block(int index, const vec3i &reject) = 0;
	virtual void fill_partial_tile(int index, const vec3i &reject) = 0;
	virtual void fill_tile(int index, const vec3i &reject) = 0;
	virtual void shade(int index, int offset, const vec3i &w) = 0;
};

template<class T>
class DataArena
{
public:
	DataArena()
	: m_buf(nullptr)
	, m_capacity(0)
	, m_tail(0)
	{}
	
	~DataArena() {
		if(m_buf)
			delete [] m_buf;
	}
	
	bool reserve(size_t size) {
		if(m_buf)
			delete [] m_buf;
		m_buf = new T[size];
		m_capacity = size;
		m_tail = 0;
	}
	
	void reset() {
		m_tail = 0;
	}
	
	T* alloc(int count=1) {
		size_t end = m_tail + count;
		if(end > m_capacity)
			return nullptr;
		T* ptr = m_buf + m_tail;
		m_tail = end;
		return ptr;
	}
private:
	T *m_buf;
	size_t m_capacity;
	size_t m_tail;
};

// 1. vertices are in counter-clockwise order
void setup_triangle(TriangleEdgeInfo *edge, const vec2f &vf0, const vec2f &vf1, const vec2f &vf2)
{
	const vec2i vi[3] = {
		snap_to_subpixel<SUB_PIXEL_PRECISION>(vf1),
		snap_to_subpixel<SUB_PIXEL_PRECISION>(vf2),
		snap_to_subpixel<SUB_PIXEL_PRECISION>(vf0),
	};
	
	constexpr int block_size_hp = BLOCK_SIZE << SUB_PIXEL_PRECISION;

	for(int i1=2, i2=0, j=0; i2 < 3; i1 = i2, i2 += 1, ++j)
	{
		auto &vi1 = vi[i1];
		auto &vi2 = vi[i2];
		vec2i rc;  // reject corner
		vec2i ac;  // offset of reject corner to accpet corner
		vec2i &dv = edge->dxdy[j];
		dv.setValue(vi1.y - vi2.y, vi2.x - vi1.x);
		int dx = vi2.x - vi1.x;
		int dy = vi2.y - vi1.y;
		if(dx >= 0) {
			if(dy >= 0) {
				// point to 1st quadrant
				// the trivial reject point is at *
				// * -- o
				// |    |
				// o -- o
				rc.setValue(0, block_size_hp);
				ac.setValue(dv.x, -dv.y);
				int r1 = ac.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ac.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge->rc_steps[j] = {
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
				rc.setValue(block_size_hp, block_size_hp);
				ac.setValue(-dv.x, -dv.y);
				int r1 = ac.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ac.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge->rc_steps[j] = {
					r3 + c3, r3 + c2, r3 + c1, r3,
					r2 + c3, r2 + c2, r2 + c1, r2,
					r1 + c3, r1 + c2, r1 + c1, r1,
					c3, c2, c1, 0,
				};
			}
		}
		else {
			if(dy >= 0) {
				// point to 2nd quadrand
				// the trivial reject point is at *
				// o -- o
				// |    |
				// * -- o
				rc.setValue(0, 0);
				ac.setValue(dv.x, dv.y);
				int r1 = ac.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ac.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge->rc_steps[j] = {
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
				rc.setValue(block_size_hp, 0);
				ac.setValue(-dv.x, dv.y);
				int r1 = ac.y;
				int r2 = r1 * 2;
				int r3 = r1 * 3;
				int c1 = ac.x;
				int c2 = c1 * 2;
				int c3 = c1 * 3;
				edge->rc_steps[j] = {
					c3, c2, c1, 0,
					r1 + c3, r1 + c2, r1 + c1, r1,
					r2 + c3, r2 + c2, r2 + c1, r2,
					r3 + c3, r3 + c2, r3 + c1, r3,
				};
			}
		} // detect edge direction
		edge->rc2ac[j] = ac.x + ac.y;
		int64_t v = edge_function_fixed(vi1, vi2, rc);
		edge->rc_hp[j] = v;
		edge->tail[j] = float(v & 0xFF) / 255;
		edge->bias[j] = is_top_left(vi1, vi2) ? 0 : -1;
	}
}

void scan_block(const TriangleEdgeInfo *edge, int row, int col, ITileBin *bin)
{
	constexpr int block_shift = SUB_PIXEL_PRECISION + BLOCK_SIZE_BITS;
	vec3i dy(edge->dxdy[0].y, edge->dxdy[1].y, edge->dxdy[2].y);
	vec3i dx(edge->dxdy[0].x, edge->dxdy[1].x, edge->dxdy[2].x);
	vec3i reject_row = {
		int(edge->rc_hp[0] >> block_shift),
		int(edge->rc_hp[1] >> block_shift),
		int(edge->rc_hp[2] >> block_shift),
	};
	vec3i reject, accept;
	for(int r = 0, i = 0; r < row; ++r, reject_row += dy)
	{
		reject = reject_row;
		for(int c = 0; c < col; ++c, ++i, reject += dx)
		{
			if((reject.x | reject.y | reject.z) < 0)
				// trivial reject
				continue;
			accept = reject + edge->rc2ac;
			if((accept.x | accept.y | accept.z) > 0) {
				// trivial accept
				bin->fill_block(i, reject);
			}
			else {
				// partial accept
				bin->fill_partial_block(i, reject);
			}
		}
	} // loop of blocks
}

void scan_tile(const TriangleEdgeInfo *edge, int index, const vec3i &reject_value, ITileBin *bin)
{
	int lod = index & LOD_MASK;
	assert(lod > 0);
	int tile_size_bits = lod * 2;
	int tile_shift = SUB_PIXEL_PRECISION + tile_size_bits;
	int tile_mask = TILE_SIZE_MASK;
	vec3i r = {
		int(edge->rc_hp[0] >> tile_shift) & tile_mask,
		int(edge->rc_hp[1] >> tile_shift) & tile_mask,
		int(edge->rc_hp[2] >> tile_shift) & tile_mask,
	};
	r.x += reject_value.x << TILE_SIZE_BITS;
	r.y += reject_value.y << TILE_SIZE_BITS;
	r.z += reject_value.z << TILE_SIZE_BITS;
	mat4i m0(r.x);
	mat4i m1(r.y);
	mat4i m2(r.z);
	m0 += edge->rc_steps[0];
	m1 += edge->rc_steps[1];
	m2 += edge->rc_steps[2];
	mat4i mask = m0 | m1;
	mask |= m2;
	int *x0 = (int*)m0.x;
	int *x1 = (int*)m1.x;
	int *x2 = (int*)m2.x;
	int *xm = (int*)mask.x;
	int istep = 1 << ((lod - 1) * 4 + LOD_BITS);
	index -= 1;
	for(int i = 0; i < 16; ++i, index += istep)
	{
		if(xm[i] < 0)
			continue;
		vec3i reject(x0[i], x1[i], x2[i]);
		vec3i accept = reject + edge->rc2ac;
		if((accept.x | accept.y | accept.z) > 0)
			bin->fill_tile(index, reject);
		else if(lod > 1)
			scan_tile(edge, index, reject, bin);
		else
			bin->fill_partial_tile(index, reject);
	}
}

void scan_pixel(const TriangleEdgeInfo *edge, int index, const vec3i &reject_value, ITileBin *bin)
{
	constexpr int pixel_center_shift = HALF_SUB_PIXEL_PRECISION;
	constexpr int sub_pixel_shift = pixel_center_shift + TILE_SIZE_BITS;
	constexpr int sub_pixel_mask = (1 << sub_pixel_shift) - 1;
	int64_t e0 = reject_value.x << sub_pixel_shift;
	int64_t e1 = reject_value.y << sub_pixel_shift;
	int64_t e2 = reject_value.z << sub_pixel_shift;
	e0 |= (edge->rc_hp[0] >> pixel_center_shift) & sub_pixel_mask;
	e1 |= (edge->rc_hp[1] >> pixel_center_shift) & sub_pixel_mask;
	e2 |= (edge->rc_hp[2] >> pixel_center_shift) & sub_pixel_mask;
	e0 += edge->rc2ac.x;
	e1 += edge->rc2ac.y;
	e2 += edge->rc2ac.z;
	mat4i m0 = int(e0 >> pixel_center_shift) + edge->bias.x;
	mat4i m1 = int(e1 >> pixel_center_shift) + edge->bias.y;
	mat4i m2 = int(e2 >> pixel_center_shift) + edge->bias.z;
	m0 += edge->rc_steps[0];
	m1 += edge->rc_steps[1];
	m2 += edge->rc_steps[2];
	mat4i mask = m0 | m1;
	mask |= m2;
	int *x0 = (int*)m0.x;
	int *x1 = (int*)m1.x;
	int *x2 = (int*)m2.x;
	int *xm = (int*)mask.x;
	for(int i = 0; i < 16; ++i, ++xm) {
		if(*xm < 0)
			continue;
		vec3f w(x0[i], x1[i], x2[i]);
		w -= edge->bias;
		w += edge->tail;
		float sum = w.x + w.y + w.z;
		w /= sum;
		bin->shade(index, i, w);
	}
}

struct Tile
{
	int index;
	vec3i reject;
	
	Tile(int i, const vec3i &v)
	: index(i), reject(v)
	{}
};

class CTilePalette : public ITileBin
{
	struct LodData
	{
		int tile_size;
		int tile_row;
		int tile_col;
		int index_shift;
		int index_mask;
		std::vector<vec2i> partial_tiles;
		std::vector<vec2i> filled_tiles;
	};
	
public:
	CTilePalette(int row, int col, int block_size)
	{
		m_block_row = row;
		m_block_col = col;
		m_block_size = block_size;
		assert(is_pod(block_size));
		int lod_count = wyc::log2p2(block_size) / 2;
		m_max_lod = lod_count - 1;
		assert(m_max_lod < 4);
		m_lod.resize(lod_count);
		int size = 4;
		int shift = 0;
		for(auto &v: m_lod) {
			v.tile_size = size;
			v.tile_row = 4;
			v.tile_col = 4;
			v.index_shift = shift;
			v.index_mask = 0xF << shift;
			size *= 4;
			shift += 4;
		}
		auto &last = m_lod.back();
		last.index_mask = ~((1 << (m_max_lod * 4)) - 1);
		last.tile_row = col;
		last.tile_col = row;
	}
	
	virtual void fill_partial_block(int index, const vec3i &reject) override
	{
		int row = index / m_block_col;
		int col = index % m_block_col;
		int x = m_block_size * col;
		int y = m_block_size * row;
		auto &lod_data = m_lod.back();
		lod_data.partial_tiles.emplace_back(x, y);
		index <<= m_max_lod * 4 + LOD_BITS;
		index += m_max_lod;
		m_partial_blocks.emplace_back(index, reject);
	}
	
	virtual void fill_block(int index, const vec3i &reject) override
	{
		int row = index / m_block_col;
		int col = index % m_block_col;
		int x = m_block_size * col;
		int y = m_block_size * row;
		auto &lod_data = m_lod.back();
		lod_data.filled_tiles.emplace_back(x, y);
	}
	
	virtual void fill_partial_tile(int index, const vec3i &reject) override
	{
		int x, y, lod;
		_get_tile_pos(index, x, y, lod);
		m_lod[lod].partial_tiles.emplace_back(x, y);
		m_partial_tiles.emplace_back(index, reject);
	}
	
	virtual void fill_tile(int index, const vec3i &reject) override
	{
		int x, y, lod;
		_get_tile_pos(index, x, y, lod);
		m_lod[lod].filled_tiles.emplace_back(x, y);
	}
	
	virtual void shade(int index, int offset, const vec3i &w) override
	{
		int x, y, lod;
		_get_tile_pos(index, x, y, lod);
		x += offset & 3;
		y += offset >> 2;
		m_shaded_pixels.emplace_back(x, y);
	}
	
	void _get_tile_pos(int index, int &x, int &y, int &lod)
	{
		x = 0, y = 0;
		lod = index & LOD_MASK;
		index >>= LOD_BITS;
		for(int l = m_max_lod; l >= lod; --l)
		{
			auto &v = m_lod[l];
			int i = (index & v.index_mask) >> v.index_shift;
			x += (i % v.tile_col) * v.tile_size;
			y += (i / v.tile_col) * v.tile_size;
		}
	}
	
	void draw_triangle(const vec2f triangle[3])
	{
		clear();
		TriangleEdgeInfo *edge = &m_edge;
		setup_triangle(edge, triangle[0], triangle[1], triangle[2]);
		scan_block(edge, m_block_row, m_block_col, this);
		for(auto &tile : m_partial_blocks) {
			scan_tile(edge, tile.index, tile.reject, this);
		}
		for(auto &tile: m_partial_tiles) {
			scan_pixel(edge, tile.index, tile.reject, this);
		}
	}
	
	void clear()
	{
		for(auto &v: m_lod) {
			v.partial_tiles.clear();
			v.filled_tiles.clear();
		}
		m_partial_blocks.clear();
		m_partial_tiles.clear();
	}
	
	int lod_count() const {
		return (int)m_lod.size();
	}
	
	void get_lod(int i, int &tile_size, const std::vector<vec2i> **fill, const std::vector<vec2i> **partial)
	{
		LodData *lod_data;
		if(i < 0 || i >= m_lod.size()) {
			tile_size = 1;
			*fill = &m_shaded_pixels;
			*partial = 0;
		}
		else {
			lod_data = &m_lod[i];
			tile_size = lod_data->tile_size;
			*fill = &lod_data->filled_tiles;
			*partial = &lod_data->partial_tiles;
		}
	}
	
private:
	int m_block_row, m_block_col;
	int m_block_size;
	int m_max_lod;
	TriangleEdgeInfo m_edge;
	std::vector<LodData> m_lod;
	std::vector<Tile> m_partial_tiles;
	std::vector<Tile> m_partial_blocks;
	std::vector<vec2i> m_shaded_pixels;
};

void draw_tile(ImDrawList* draw_list, float x, float y, float tile_size, float scale, ImColor color, const std::vector<vec2i> *tile_list)
{
	ImVec2 va, vb, vc, vd;
	tile_size *= scale;
	for(auto &v: *tile_list)
	{
		va.x = x + v.x * scale; va.y = y - v.y * scale;
		vb.x = va.x + tile_size; vb.y = va.y;
		vc.x = vb.x; vc.y = vb.y - tile_size;
		vd.x = va.x; vd.y = vc.y;
		draw_list->AddQuadFilled(va, vb, vc, vd, color);
	}
}

struct ContextOneTriangle
{
	vec2f triangle[3];
	CTilePalette *palette;
	int show_lod;
};

static ContextOneTriangle* get_demo_context()
{
	static ContextOneTriangle s_context;
	return &s_context;
}

void demo_one_triangle()
{
	auto context = get_demo_context();

	auto &triangle = context->triangle;
	triangle[0] = {6.9f, 25.1f};
	triangle[1] = {24.5f, 4.8f};
	triangle[2] = {48.6f, 37.3f};

	CTilePalette *palette = new CTilePalette(4, 4, 16);
	context->palette = palette;
	context->show_lod = -1;
	palette->draw_triangle(triangle);
}

void draw_one_triangle()
{
	constexpr int row = 64, col = 64;
	constexpr float cell_size = 11.2f;
	constexpr float cell_thickness = 1.0f;
//	constexpr float point_size = 2.0f;
	constexpr float canvas_width = 1280.0f, canvas_height = 720.0f;
	
	auto context = get_demo_context();
	auto &triangles = context->triangle;
	
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
	
	/*
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
	 */
	
	float left = origin_x;
	float bottom = origin_y + cell_size * row + cell_thickness * 0.5;
	
	if(ImGui::IsKeyReleased('1'))
	{
		context->show_lod = context->palette->lod_count();
	}
	else if(ImGui::IsKeyReleased('2'))
	{
		context->show_lod = context->palette->lod_count() - 1;
	}
	else if(ImGui::IsKeyReleased('3'))
	{
		context->show_lod = context->palette->lod_count() - 2;
	}
	
	if(context->show_lod >= 0) {
		int tile_size;
		const std::vector<vec2i> *fill_list = nullptr, *partial_list = nullptr;
		int lod = context->show_lod	- 1;
		context->palette->get_lod(lod, tile_size, &fill_list, &partial_list);
		if(partial_list)
			draw_tile(draw_list, left, bottom, tile_size, cell_size, 0x8000FFFF, partial_list);
		if(fill_list)
			draw_tile(draw_list, left, bottom, tile_size, cell_size, 0x8000FF00, fill_list);
		for(int l = context->palette->lod_count() - 1; l > lod; --l) {
			context->palette->get_lod(l, tile_size, &fill_list, &partial_list);
			if(fill_list)
				draw_tile(draw_list, left, bottom, tile_size, cell_size, 0x8000C000, fill_list);
		}
	}

	ImVec2 tri[3] = {
		{left + cell_size * triangles[0].x, bottom - cell_size * triangles[0].y},
		{left + cell_size * triangles[1].x, bottom - cell_size * triangles[1].y},
		{left + cell_size * triangles[2].x, bottom - cell_size * triangles[2].y},
	};

	draw_list->AddLine(tri[0], tri[1], 0xFFFF0000);
	draw_list->AddLine(tri[1], tri[2], 0xFFFF0000);
	draw_list->AddLine(tri[2], tri[0], 0xFFFF0000);
	
	draw_list->PopClipRect();
}
