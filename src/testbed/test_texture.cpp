#pragma once
#include "test.h"
#include "vecmath.h"
#include "mesh.h"
#include "mtl_diffuse.h"
#include "mtl_color.h"

class CTestTexture : public CTest
{
public:
	virtual void run() {
		// create mesh
		auto mesh = std::make_shared<wyc::CMesh>();
		mesh->create_uv_box(1);
		// setup transform
		wyc::mat4f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 100);
		wyc::mat4f rx_world, ry_world, transform_world;
		wyc::set_rotate_y(ry_world, wyc::deg2rad(60));
		wyc::set_rotate_x(rx_world, wyc::deg2rad(30));
		wyc::mat4f proj_from_world;
		// sampler
		auto diffuse_img = std::make_shared<wyc::CImage>();
		if (!diffuse_img->load("res/checkerboard.png")) {
			return;
		}
		auto sampler = std::make_shared<wyc::CSpwSampler>(diffuse_img.get());

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		auto mtl = std::make_shared<CMaterialDiffuse>();
		wyc::set_translate(transform_world, 0, 0, -5);
		proj_from_world = proj * transform_world * rx_world * ry_world;
		mtl->set_uniform("proj_from_world", proj_from_world);
		mtl->set_uniform("diffuse", (wyc::CSampler*)sampler.get());
		draw->material = mtl.get();
		m_renderer->enqueue(draw);

		m_renderer->process();
		save_image("textured_box.png");

	}
};
REGISTER_TEST(CTestTexture)