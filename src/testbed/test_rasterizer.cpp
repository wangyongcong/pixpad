#include <chrono>
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_color.h"
#include "surface.h"
#include "spw_rasterizer.h"
#include "ImathBoxAlgo.h"
#include "renderer/image.h"
#include "metric.h"
#include "spw_tile.h"

enum ERasterizerType
{
	RT_LINEAR,
	RT_LARRABEE,
	
	RT_COUNT
};

using namespace wyc;

inline vec3f viewport_tranform(const vec4f &pos, const vec3f &translate, const vec3f &scale)
{
	vec3f v(pos.x / pos.w, pos.y / pos.w, pos.z / pos.w);
	return translate + v * scale;
}

class CTestRasterizer : public CTest
{
public:
	virtual bool setup_renderer(unsigned img_w, unsigned img_h, unsigned max_core=0) override
	{
		img_w = align_up(img_w, 64);
		img_h = align_up(img_h, 64);
		m_ldr_image.storage(img_w, img_h, 4);
		m_ldr_image.clear(0xFF000000);
		m_depth.storage(img_w, img_h, 4);
		m_depth.clear(1.0f);

		std::string ply_file;
		if (!get_param("model", ply_file)) {
			log_error("no model");
			return false;
		}
		std::string param;
		m_type = RT_LINEAR;
		if(get_param("type", param)) {
			try {
				m_type = (ERasterizerType)std::stoi(param);
			}
			catch(const std::invalid_argument&) {
				log_error("invalid raterizer type: %s", param);
			}
			if(m_type >= RT_COUNT) {
				m_type = RT_LINEAR;
			}
		}
		log_info("rasterizer type: %d", m_type);
		m_mesh = std::make_shared<wyc::CMesh>();
		if (!m_mesh->load(ply_file)) {
			return false;
		}
		setup_viewport();
		log_info("init ok");
		return true;
	}
	
	void setup_viewport()
	{
		// setup viewport
		int canvas_w = (int)m_image_w, canvas_h = (int)m_image_h;
		m_viewport_translate = {canvas_w * 0.5f, canvas_h * 0.5f, 0};
		m_viewport_scale = {canvas_w * 0.5f, canvas_h * 0.5f, 1};
		m_viewport_bounding = {{0, 0}, {canvas_w, canvas_h}};
		constexpr int block_size = 64;
		int block_row = (canvas_h + block_size - 1) / block_size;
		int block_col = (canvas_w + block_size - 1) / block_size;
		m_block_boundings.reserve(block_row * block_col);
		for(int r = 0; r < block_row; ++r) {
			for(int c = 0; c < block_col; ++c) {
				int x = c * block_size, y = r * block_size;
				m_block_boundings.emplace_back(vec2i(x, y), vec2i(x + block_size, y + block_size));
			}
		}
	}
	
	virtual void run() override
	{
		// setup transform
		wyc::mat4f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 100);
		wyc::mat4f rx_world, translate_world;
		wyc::set_rotate_x(rx_world, wyc::deg2rad(45));
		wyc::set_translate(translate_world, 0, 0, -4);
		wyc::mat4f proj_from_world;
		proj_from_world = proj * translate_world * rx_world;
		
		// process vertices
		const auto &vb = m_mesh->vertex_buffer();
		const auto &ib = m_mesh->index_buffer();
		size_t vcount = vb.size();
		std::vector<vec4f> vertices;
		vertices.reserve(vcount);
		assert(vb.attrib_component(ATTR_POSITION) == 3);

		{
			TIME_VERTEX_SHADER
			auto pos_array = vb.get_attribute(ATTR_POSITION);
			for(auto it = pos_array.begin(), end = pos_array.end(); it != end; ++it) {
				COUNT_VERTEX
				vec3f v3 = *it;
				vec4f v4(v3);
				vertices.emplace_back(proj_from_world * v4);
			}
		}

		const vec3i *indices = (const vec3i*)ib.data();
		int triangle_count = (int)(ib.size() / 3);
		m_min_z = 1.0f;
		m_max_z = -1.0f;

		if(m_type == RT_LARRABEE) {
			draw_triangle_larrabee(triangle_count, indices, vertices);
			save_image("bin/torus_larrabee.png");
		}
		else {
			draw_triangles(triangle_count, indices, vertices);
			save_image("bin/torus.png");
		}
		wyc::CSpwMetric::singleton()->report();
		log_info("z range: [%f, %f]", m_min_z, m_max_z);

	}
	
	void draw_triangles(unsigned triangle_count, const vec3i *indices, const std::vector<vec4f> &vertices)
	{
		for(unsigned i = 0; i < triangle_count; i += 1)
		{
			const vec3i &index = indices[i];
			const auto &v0 = vertices[index.x];
			const auto &v1 = vertices[index.y];
			const auto &v2 = vertices[index.z];
			// backface culling
			if(is_backface(v0, v1, v2)) {
				BACKFACE_CULLING
				continue;
			}
			// viewport transform
			vec3f pos[3];
			pos[0] = viewport_tranform(v0, m_viewport_translate, m_viewport_scale);
			pos[1] = viewport_tranform(v1, m_viewport_translate, m_viewport_scale);
			pos[2] = viewport_tranform(v2, m_viewport_translate, m_viewport_scale);
			// bounding test
			Imath::Box2i bounding;
			Imath::bounding(bounding, pos, pos+1, pos+2);
			Imath::intersection(bounding, m_viewport_bounding);
			if(bounding.isEmpty()) {
				VIEWPORT_CULLING
				continue;
			}
			COUNT_TRIANGLE
			TIME_DRAW_TRIANGLE
			for(auto b : m_block_boundings) {
				Imath::intersection(b, bounding);
				if(!b.isEmpty()) {
					fill_triangle(b, pos[0], pos[1], pos[2], *this);
				}
			} // block_boundings
		} // triangle_coung
	}

	void scan_partial_queue(const Triangle *prim, BlockArena *arena, TileQueue *queue, TileQueue *full_tiles, TileQueue *partial_tiles) {
		TileQueue output;
		while(queue->head) {
			TileBlock *t = queue->pop();
			scan_tile(prim, t, arena, full_tiles, &output);
			arena->free(t);
			if(output.head) {
				if(output.head->lod < SPW_LOD_MAX)
					scan_partial_queue(prim, arena, &output, full_tiles, partial_tiles);
				else
					partial_tiles->join(&output);
			}
		}
	}
	
	void scan_full_queue(const Triangle *prim, BlockArena *arena, TileQueue *queue, PixelShader shader)
	{
		while(queue->head) {
			TileBlock *t = queue->pop();
			if(t->lod < SPW_LOD_MAX) {
				split_tile(prim, arena, t, queue);
			}
			else {
				fill_tile(prim, t, shader);
			}
			arena->free(t);
		}
	}
	
	void draw_triangle_larrabee(unsigned triangle_count, const vec3i *indices, const std::vector<vec4f> &vertices)
	{
		BlockArena arena(256);
		unsigned max_bucket_count = 1;
		unsigned max_block_count = 0;
		RenderTarget rt;
		rt.storage = (char*)m_ldr_image.get_buffer();
		rt.pixel_size = m_ldr_image.fragment_size();
		rt.pitch = m_ldr_image.pitch();
		rt.w = m_ldr_image.row_length();
		rt.h = m_ldr_image.row();
		rt.x = 0;
		rt.y = 0;
		
		auto shader = [](char *dst, const vec3f &w) {
			COUNT_PIXEL
			*((unsigned*)dst) = 0xFF00FF00;
		};

		for(unsigned i = 0; i < triangle_count; i += 1)
		{
			const vec3i &index = indices[i];
			const auto &v0 = vertices[index.x];
			const auto &v1 = vertices[index.y];
			const auto &v2 = vertices[index.z];
			// backface culling
			if(is_backface(v0, v1, v2)) {
				BACKFACE_CULLING
				continue;
			}
			// viewport transform
			vec3f pos[3];
			pos[0] = viewport_tranform(v0, m_viewport_translate, m_viewport_scale);
			pos[1] = viewport_tranform(v1, m_viewport_translate, m_viewport_scale);
			pos[2] = viewport_tranform(v2, m_viewport_translate, m_viewport_scale);
			// draw triangle
			COUNT_TRIANGLE
			{
				TIME_DRAW_TRIANGLE
				Triangle prim;
				setup_triangle(&prim, (const vec2f*)pos, (const vec2f*)(pos+1), (const vec2f*)(pos+2));
				TileQueue partial_blocks, full_tiles, partial_tiles;
				auto block_count = scan_block(&rt, &prim, &arena, &full_tiles, &partial_blocks);
				if (block_count > max_block_count)
					max_block_count = block_count;
				scan_partial_queue(&prim, &arena, &partial_blocks, &full_tiles, &partial_tiles);
				scan_full_queue(&prim, &arena, &full_tiles, shader);
				for(auto it = partial_tiles.head; it; it = it->_next)
				{
					draw_tile(&prim, it, shader);
				}
				if(arena.bucket_count() > max_bucket_count) {
					max_bucket_count = arena.bucket_count();
				}
				arena.clear();
			}
		}
		log_info("max bucket count: %d", max_bucket_count);
		log_info("max block count: %d", max_block_count);
	}
	
	void operator() (int x, int y, float z, float w0, float w1, float w2)
	{
/*		TIME_PIXEL_SHADER
		if(z < m_min_z)
			m_min_z = z;
		if(z > m_max_z)
			m_max_z = z;
		float depth = *m_depth.get<float>(x, y);
		if(z >= depth) {
			DEPTH_CULLING
			return;
		}
		COUNT_PIXEL
		m_depth.set(x, y, z);
		unsigned c = (int((1.0f - z) * 255) << 8) | 0xFF000000;
		m_ldr_image.set(x, y, c);
 */
		COUNT_PIXEL
		m_ldr_image.set(x, y, 0xFF00FF00);
	}
	
	void save_image(const char *name)
	{
		CImage *image = nullptr;
		if(m_type == RT_LARRABEE) {
			image = new CImage();
			image->create_empty(m_image_w, m_image_h);
			uint32_t *data = (uint32_t*)image->buffer();
			linearize_32bpp(data, m_image_w, m_image_h, m_image_w, (uint32_t*)m_ldr_image.get_buffer(), 0, 0, m_ldr_image.row_length(), m_ldr_image.row());
		}
		else {
			image = new CImage(m_ldr_image.get_buffer(), m_image_w, m_image_h, m_ldr_image.pitch());
		}
		if (m_outfile.empty()) {
			m_outfile = name;
		}
		if (!image->save(m_outfile))
		{
			log_error("Failed to save image file");
		}
		delete image;
	}

private:
	std::shared_ptr<wyc::CMesh> m_mesh;
	CSurface m_depth;
	// viewport settings
	vec3f m_viewport_translate;
	vec3f m_viewport_scale;
	Imath::Box2i m_viewport_bounding;
	std::vector<Imath::Box2i> m_block_boundings;
	float m_min_z, m_max_z;
	ERasterizerType m_type;
};

REGISTER_TEST(CTestRasterizer)
