#pragma once
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_wireframe.h"

class CTestWireframe : public CTest
{
public:
	virtual void run() 
	{
		std::string ply_file;
		if (!get_param("model", ply_file)) {
			log_error("no model");
			return;
		}
		auto mesh = std::make_shared<wyc::CMesh>();
		if (!mesh->load_ply(ply_file)) {
			return;
		}

		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 1000);
		Imath::M44f rx_world, ry_world, transform_world;
		wyc::set_rotate_y(ry_world, wyc::deg2rad(45));
		wyc::set_rotate_x(rx_world, wyc::deg2rad(15));
		Imath::M44f proj_from_world;

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		auto mtl = std::make_shared<CMaterialWireframe>();
		wyc::set_translate(transform_world, 0, 0, -2.5f);
		proj_from_world = proj * transform_world * ry_world * rx_world;
		mtl->set_uniform("proj_from_world", proj_from_world);
		mtl->set_uniform("line_color", Imath::C4f{ 0, 1, 0, 1 });
		mtl->set_uniform("fill_color", Imath::C4f{ 0.2f, 0.2f, 0.2f, 1 });
		draw->material = mtl.get();
		m_renderer->enqueue(draw);

		m_renderer->process();
		save_image("isosahedron.png");
	}

private:
};

REGISTER_NEW_TEST(CTestWireframe)
