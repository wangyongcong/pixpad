#pragma once
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_wireframe.h"
#include "mtl_lambert.h"

class CTestLambert : public CTest
{
public:
	virtual void run()
	{
		auto mesh = std::make_shared<wyc::CMesh>();
		mesh->create_sphere(1.0f);

		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 0.1f, 10000);
		Imath::M44f transform_world;
		wyc::set_translate(transform_world, 0, 0, -10.0f);
		Imath::M44f proj_from_world = proj * transform_world;

		// setup material
		auto mtl = std::make_shared<CMaterialWireframe>();
		mtl->set_uniform("proj_from_world", proj);
		mtl->set_uniform("line_color", Imath::C4f{ 0, 1, 0, 1 });
		mtl->set_uniform("fill_color", Imath::C4f{ 0.2f, 0.2f, 0.2f, 1 });

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		draw->material = mtl.get();

		m_renderer->enqueue(draw);
		m_renderer->process();
		save_image("lambert.png");
	}
};

REGISTER_TEST(CTestLambert)