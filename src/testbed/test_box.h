#pragma once
#include <cmath>
#include "boost/filesystem.hpp"
#include "test.h"
#include "mesh.h"
#include "vecmath.h"
#include "mtl_color.h"
#include "metric.h"

class CTestBox : public CTest
{
public:
	static CTest* create() {
		return new CTestBox();
	}

	virtual void run() {
		// create mesh
		wyc::CMesh *mesh = new wyc::CMesh();
		mesh->create_box(1);
		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 100);
		Imath::M44f mrx, mry, mt;
		wyc::set_rotate_y(mry, wyc::deg2rad(60));
		wyc::set_rotate_x(mrx, wyc::deg2rad(30));
		Imath::M44f mvp;

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh;
		auto *mtl = new wyc::CMaterialColor();
		wyc::set_translate(mt, 0, 0, -5);
		mvp = proj * mt * mrx * mry;
		mtl->set_uniform("mvp_matrix", mvp);
		mtl->set_uniform("color", Imath::C4f{ 0, 1, 0, 1 });
		draw->material = mtl;
		m_renderer->enqueue(draw);

		if (has_param("two")) {
			std::string s;
			Imath::C4f color_two;
			if (get_param("color", s)) {
				auto v = std::strtoul(s.c_str(), 0, 16);
				color_two = { (v >> 24) / 255.0f, ((v >> 16) & 0xFF) / 255.0f, ((v >> 8) & 0xFF) / 255.0f, (v & 0xFF) / 255.0f };
			}
			else {
				color_two = { 1, 0, 0, 1 };
			}

			auto draw2 = m_renderer->new_command<wyc::cmd_draw_mesh>();
			draw2->mesh = mesh;
			auto *mtl2 = new wyc::CMaterialColor();
			wyc::set_translate(mt, 0, 1, -6);
			mvp = proj * mt * mrx * mry;
			mtl2->set_uniform("mvp_matrix", mvp);
			mtl2->set_uniform("color", color_two);
			draw2->material = mtl2;
			m_renderer->enqueue(draw2);
		}

		TIMER_BEG(1)
			m_renderer->process();
		TIMER_END
		wyc::CSpwMetric::singleton()->report();

		save_image("box.png");
	}
};
