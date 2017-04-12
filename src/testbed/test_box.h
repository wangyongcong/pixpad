#pragma once
#include <cmath>
#include "boost/filesystem.hpp"
#include "test.h"
#include "image.h"
#include "spw_renderer.h"
#include "mesh.h"
#include "vecmath.h"
#include "spw_pipeline_wireframe.h"
#include "mtl_flat_color.h"
#include "metric.h"

class CTestBox : public CTest
{
public:
	static CTest* create() {
		return new CTestBox();
	}

	virtual void run() {
		printf("draw box");
		unsigned img_w = 960, img_h = 540;
		if(m_outfile.empty())
			m_outfile = "box.png";
		// create mesh
		wyc::CMesh *mesh = new wyc::CMesh();
		mesh->create_box(1);
		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(img_w) / img_h, 1, 100);
		Imath::M44f mrx, mry, mt;
		wyc::set_rotate_y(mry, wyc::deg2rad(60));
		wyc::set_rotate_x(mrx, wyc::deg2rad(30));
		Imath::M44f mvp;
		// setup pipeline
		auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
		render_target->create(img_w, img_h, wyc::SPR_COLOR_R8G8B8A8 | wyc::SPR_DEPTH_32);
		auto renderer = std::make_shared<wyc::CSpwRenderer>();
		renderer->set_render_target(render_target);
		//auto pipeline = std::make_shared<wyc::CSpwPipelineWireFrame>();
		auto pipeline = std::make_shared<wyc::CSpwPipeline>();
		pipeline->setup();
		renderer->set_pipeline(pipeline);
		// clear
		auto clr = renderer->new_command<wyc::cmd_clear>();
		clr->color = { 0.0f, 0.0f, 0.0f };
		renderer->enqueue(clr);

		auto draw = renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh;
		auto *mtl = new wyc::CMaterialFlatColor();
		wyc::set_translate(mt, 0, 0, -5);
		mvp = proj * mt * mrx * mry;
		mtl->set_uniform("mvp_matrix", mvp);
		mtl->set_uniform("color", Imath::C4f{ 0, 1, 0, 1 });
		draw->material = mtl;
		renderer->enqueue(draw);

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

			auto draw2 = renderer->new_command<wyc::cmd_draw_mesh>();
			draw2->mesh = mesh;
			auto *mtl2 = new wyc::CMaterialFlatColor();
			wyc::set_translate(mt, 0, 1, -6);
			mvp = proj * mt * mrx * mry;
			mtl2->set_uniform("mvp_matrix", mvp);
			mtl2->set_uniform("color", color_two);
			draw2->material = mtl2;
			renderer->enqueue(draw2);
		}

		TIMER_BEG(1)
		renderer->process();
		TIMER_END
		wyc::CSpwMetric::singleton()->report();

		auto &buffer = render_target->get_color_buffer();
		wyc::CImage image(buffer.get_buffer(), buffer.row_length(), buffer.row(), buffer.pitch());
		if (!image.save(m_outfile))
		{
			log_error("Failed to save image file");
		}
	}
};
