#pragma once
#include <cmath>
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
		std::string img_file = "box.png";
		// create mesh
		wyc::CMesh *mesh = new wyc::CMesh();
		mesh->create_box(1);
		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(img_w) / img_h, 1, 100);
		Imath::M44f mrx, mry, mt;
		wyc::set_rotate_y(mry, wyc::deg2rad(60));
		wyc::set_rotate_x(mrx, wyc::deg2rad(30));
		wyc::set_translate(mt, 0, 0, -5);
		Imath::M44f mvp = proj * mt * mrx * mry;
		// material
		auto *mtl = new wyc::CMaterialFlatColor();
		mtl->set_uniform("mvp_matrix", mvp);
		mtl->set_uniform("color", Imath::C4f{ 0, 1, 0, 1 });
		// setup pipeline
		auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
		render_target->create(img_w, img_h, wyc::SPR_COLOR_R8G8B8A8);
		auto renderer = std::make_shared<wyc::CSpwRenderer>();
		renderer->set_render_target(render_target);
		auto pipeline = std::make_shared<wyc::CSpwPipelineWireFrame>();
		renderer->set_pipeline(pipeline);
		// draw
		auto clr = renderer->new_command<wyc::cmd_clear>();
		clr->color = { 0.0f, 0.0f, 0.0f };
		renderer->enqueue(clr);
		auto draw = renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh;
		draw->material = mtl;
		renderer->enqueue(draw);
		TIMER_BEG(1)
		renderer->process();
		TIMER_END
		wyc::CSpwMetric::singleton()->report();
		auto &buffer = render_target->get_color_buffer();
		wyc::CImage image(buffer.get_buffer(), buffer.row_length(), buffer.row(), buffer.pitch());
		if (!image.save(img_file))
		{
			log_error("Failed to save image file");
		}
	}
};
