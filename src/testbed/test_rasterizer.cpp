#include <chrono>
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_color.h"
#include "surface.h"
#include "spw_rasterizer.h"
#include "ImathBoxAlgo.h"
#include "image.h"
#include "metric.h"

using namespace wyc;

inline vec3f viewport_tranform(const vec4f &pos, const vec3f &translate, const vec3f &scale)
{
	vec3f v(pos.x / pos.w, pos.y / pos.w, pos.z / pos.w);
	return translate + v * scale;
}

class CTestRasterizer : public CTest
{
public:
	virtual void setup_renderer(unsigned img_w, unsigned img_h, unsigned max_core=0) override
	{
		m_ldr_image.storage(img_w, img_h, 4);
		m_ldr_image.clear(0xFF000000);
		m_depth.storage(img_w, img_h, 4);
		m_depth.clear(1.0f);

		std::string ply_file;
		if (!get_param("model", ply_file)) {
			log_error("no model");
			return;
		}
		m_mesh = std::make_shared<wyc::CMesh>();
		if (!m_mesh->load_ply(ply_file)) {
			return;
		}
		setup_viewport();
		log_info("init ok");
	}
	
	void setup_viewport()
	{
		// setup viewport
		int canvas_w = (int)m_image_w, canvas_h = (int)m_image_h;
		m_viewport_translate = {canvas_w * 0.5f, canvas_h * 0.5f, 0};
		m_viewport_scale = {canvas_w * 0.5f, canvas_h * 0.5f, 1};
		m_viewport_bounding = {{0, 0}, {canvas_w, canvas_h}};
		constexpr int block_size = 64;
		int block_row = canvas_h / block_size;
		int block_col = canvas_w / block_size;
		m_block_boundings.reserve(block_row * block_col);
		for(int r = 0; r < block_row; ++r) {
			for(int c = 0; c < block_col; ++c) {
				int x = r * block_size, y = c * block_size;
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
		
		draw_triangles(triangle_count, indices, vertices);

		wyc::CSpwMetric::singleton()->report();
		log_info("z range: [%f, %f]", m_min_z, m_max_z);

//		save_image("bin/torus.png");
	}
	
	void draw_triangles(unsigned triangle_count, const vec3i *indices, const std::vector<vec4f> &vertices)
	{
		for(int i = 0; i < triangle_count; i += 1)
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
			for(auto b : m_block_boundings) {
				Imath::intersection(b, bounding);
				if(!b.isEmpty()) {
					COUNT_TRIANGLE
					TIME_DRAW_TRIANGLE
					fill_triangle(b, pos[0], pos[1], pos[2], *this);
				}
			}
		}
	}
	
	void draw_triangle_larrabee(unsigned triangle_count, const vec3i *indices, const std::vector<vec4f> &vertices)
	{
		
		
	}
	
	void operator() (int x, int y, float z, float w0, float w1, float w2)
	{
		TIME_PIXEL_SHADER
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
	}
	
	void save_image(const char *name)
	{
		CImage image(m_ldr_image.get_buffer(), m_image_w, m_image_h, m_ldr_image.pitch());
		if (m_outfile.empty()) {
			m_outfile = name;
		}
		if (!image.save(m_outfile))
		{
			log_error("Failed to save image file");
		}
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
};

REGISTER_TEST(CTestRasterizer)
