#pragma once
#include "test.h"
#include "vecmath.h"
#include "mesh.h"
#include "mtl_diffuse.h"
#include "mtl_color.h"

class CTestTexture : public CTest
{
public:
	static CTest* create() {
		return new CTestTexture();
	}

	virtual void run() {
		// create mesh
		auto mesh = std::make_shared<wyc::CMesh>();
		mesh->create_uv_box(1);
		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 100);
		Imath::M44f mrx, mry, mt;
		wyc::set_rotate_y(mry, wyc::deg2rad(60));
		wyc::set_rotate_x(mrx, wyc::deg2rad(30));
		Imath::M44f mvp;
		// sampler
		auto diffuse_img = std::make_shared<wyc::CImage>();
		if (!diffuse_img->load("res/checkerboard.png")) {
			return;
		}
		auto sampler = std::make_shared<wyc::CSpwSampler>(diffuse_img.get());

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		auto mtl = std::make_shared<wyc::CMaterialDiffuse>();
		wyc::set_translate(mt, 0, 0, -5);
		mvp = proj * mt * mrx * mry;
		mtl->set_uniform("mvp_matrix", mvp);
		mtl->set_uniform("diffuse", (wyc::CSampler*)sampler.get());
		draw->material = mtl.get();
		m_renderer->enqueue(draw);

		m_renderer->process();
		save_image("textured_box.png");

	}
};