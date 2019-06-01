#include <cmath>
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_color.h"
#include "metric.h"

class CTestBox : public CTest
{
public:
	virtual void run() {
		// create mesh
		wyc::CMesh *mesh = new wyc::CMesh();
		mesh->create_box(1);
		// setup transform
		wyc::mat4f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 100);
		wyc::mat4f rx_world, ry_world, transform_world;
		wyc::set_rotate_y(ry_world, wyc::deg2rad(60));
		wyc::set_rotate_x(rx_world, wyc::deg2rad(30));
		wyc::mat4f proj_from_world;

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh;
		auto *mtl = new CMaterialColor();
		wyc::set_translate(transform_world, 0, 0, -5);
		proj_from_world = proj * transform_world * rx_world * ry_world;
		mtl->set_uniform("proj_from_world", proj_from_world);
		mtl->set_uniform("color", wyc::color4f{ 0, 1, 0, 1 });
		draw->material = mtl;
		m_renderer->enqueue(draw);

		if (has_param("two")) {
			std::string s;
			wyc::color4f color_two;
			if (get_param("color", s)) {
				auto v = std::strtoul(s.c_str(), 0, 16);
				color_two = { (v >> 24) / 255.0f, ((v >> 16) & 0xFF) / 255.0f, ((v >> 8) & 0xFF) / 255.0f, (v & 0xFF) / 255.0f };
			}
			else {
				color_two = { 1, 0, 0, 1 };
			}

			auto draw2 = m_renderer->new_command<wyc::cmd_draw_mesh>();
			draw2->mesh = mesh;
			auto *mtl2 = new CMaterialColor();
			wyc::set_translate(transform_world, 0, 1, -6);
			proj_from_world = proj * transform_world * rx_world * ry_world;
			mtl2->set_uniform("proj_from_world", proj_from_world);
			mtl2->set_uniform("color", color_two);
			draw2->material = mtl2;
			m_renderer->enqueue(draw2);
		}

		m_renderer->process();
		save_image("box.png");
	}
};

REGISTER_TEST(CTestBox)
