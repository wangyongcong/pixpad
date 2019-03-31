#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_color.h"
#include "surface.h"
#include "spw_rasterizer.h"
#include "ImathBoxAlgo.h"
#include "image.h"

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
		
		log_info("init ok");
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
		
		// setup viewport
		int canvas_w = (int)m_image_w, canvas_h = (int)m_image_h;
		vec3f vp_translate = {canvas_w * 0.5f, canvas_h * 0.5f, 0};
		vec3f vp_scale = {canvas_w * 0.5f, canvas_h * 0.5f, 1};
		Imath::Box2i vp_bounding = {
			{0, 0}, {canvas_w, canvas_h}
		};

		// process vertices
		const auto &vb = m_mesh->vertex_buffer();
		const auto &ib = m_mesh->index_buffer();
		size_t vcount = vb.size();
		std::vector<vec4f> vertices;
		vertices.reserve(vcount);
		assert(vb.attrib_component(ATTR_POSITION) == 3);
		auto pos_array = vb.get_attribute(ATTR_POSITION);
		for(auto it = pos_array.begin(), end = pos_array.end(); it != end; ++it) {
			vec3f v3 = *it;
			vec4f v4(v3);
			vertices.emplace_back(proj_from_world * v4);
		}
		const vec3i *indices = (const vec3i*)ib.data();
		unsigned triangle_count = ib.size() / 3;
		unsigned draw_count = 0;
		unsigned backface_count = 0;
		unsigned outside_count = 0;
		constexpr int block_size = 64;
		int block_row = canvas_h / block_size;
		int block_col = canvas_w / block_size;
		std::vector<Imath::Box2i> block_boundings;
		block_boundings.reserve(block_row * block_col);
		for(int r = 0; r < block_row; ++r) {
			for(int c = 0; c < block_col; ++c) {
				int x = r * block_size, y = c * block_size;
				block_boundings.emplace_back(vec2i(x, y), vec2i(x + block_size, y + block_size));
			}
		}

		m_pixel_count = 0;
		m_shade_count = 0;
		m_min_z = 1.0f;
		m_max_z = -1.0f;
		for(int i = 0; i < triangle_count; i += 1)
		{
			const vec3i &index = indices[i];
			const auto &v0 = vertices[index.x];
			const auto &v1 = vertices[index.y];
			const auto &v2 = vertices[index.z];
			// backface culling
			if(is_backface(v0, v1, v2)) {
				backface_count += 1;
				continue;
			}
			// viewport transform
			vec3f pos[3];
			pos[0] = viewport_tranform(v0, vp_translate, vp_scale);
			pos[1] = viewport_tranform(v1, vp_translate, vp_scale);
			pos[2] = viewport_tranform(v2, vp_translate, vp_scale);
			// bounding test
			Imath::Box2i bounding;
			Imath::bounding(bounding, pos, pos+1, pos+2);
			Imath::intersection(bounding, vp_bounding);
			if(bounding.isEmpty()) {
				outside_count += 1;
				continue;
			}
			for(auto b : block_boundings) {
				Imath::intersection(b, bounding);
				if(!b.isEmpty()) {
					fill_triangle(b, pos[0], pos[1], pos[2], *this);
				}
			}
			// draw triangle
			draw_count += 1;
		}
		log_info("cull backface: %d", backface_count);
		log_info("outside viewport: %d", outside_count);
		log_info("draw triangles: %d/%d", draw_count, triangle_count);
		log_info("draw pixel: %d/%d", m_shade_count, m_pixel_count);
		log_info("z range: [%f, %f]", m_min_z, m_max_z);
		save_image("torus.png");
	}
	
	void operator() (int x, int y, float z, float w0, float w1, float w2)
	{
		m_pixel_count += 1;
		if(z < m_min_z)
			m_min_z = z;
		if(z > m_max_z)
			m_max_z = z;
		float depth = *m_depth.get<float>(x, y);
		if(z < depth) {
			m_shade_count += 1;
			m_depth.set(x, y, z);
			unsigned c = (int((1.0f - z) * 255) << 8) | 0xFF000000;
			m_ldr_image.set(x, y, c);
		}
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
	unsigned m_pixel_count;
	unsigned m_shade_count;
	float m_min_z, m_max_z;
};

REGISTER_TEST(CTestRasterizer)
