#pragma once
#include "stdafx.h"
#include "scene.h"
#include "spw_renderer.h"
#include "image.h"
#include "mtl_flat_color.h"
#include "spw_pipeline_wireframe.h"
#include "test.h"


class CTestCollada : public CTest
{
	std::string m_img_file;
	std::string m_scn_file;
	bool m_is_wireframe;
public:
	static CTest* create() {
		return new CTestCollada();
	}
	CTestCollada() {
		m_scn_file = "res/cube.dae";
		m_img_file = "test_collada.png";
		m_is_wireframe = true;
	}
	virtual void init(int argc, char *argv[])
	{
		//TCLAP::CmdLine cmd("Generate image using Sparrow renderer.", ' ', "0.0.1");
		//TCLAP::UnlabeledValueArg<std::string> input_file("input_file", "Input scene file", true, "", "path string");
		//cmd.add(input_file);
		//TCLAP::UnlabeledValueArg<std::string> output_file("output_file", "Output image file", false, "", "path string");
		//cmd.add(output_file);
		//TCLAP::SwitchArg opt_wireframe("w", "wireframe", "Enable wireframe mode", cmd);
		//cmd.parse(argc, argv);
		//m_scn_file = input_file.getValue();
		//m_img_file = output_file.getValue();
		//m_is_wireframe = opt_wireframe.getValue();
		//if (m_img_file.empty())
		//{
		//	size_t end = m_scn_file.rfind(".");
		//	if (end == std::string::npos)
		//		m_img_file = m_scn_file + ".png";
		//	else {
		//		std::string base_name = m_scn_file.substr(0, end);
		//		m_img_file = base_name + ".png";
		//	}
		//}
	}
	virtual void run()
	{
		unsigned core_count = std::thread::hardware_concurrency();
		log_debug("max thread count: %d", core_count);
		auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
		render_target->create(960, 540, wyc::SPR_COLOR_R8G8B8A8);
		auto renderer = std::make_shared<wyc::CSpwRenderer>();
		renderer->set_render_target(render_target);
		std::shared_ptr<wyc::CSpwPipeline> pipeline;
		if (m_is_wireframe)
			pipeline = std::make_shared<wyc::CSpwPipelineWireFrame>();
		else
			pipeline = std::make_shared<wyc::CSpwPipeline>();
		renderer->set_pipeline(pipeline);
		wyc::CScene scn;
		std::wstring w_scn_file;
		wyc::CCamera camera;
		if (!wyc::str2wstr(w_scn_file, m_scn_file))
		{
			log_error("Invalid file path");
			return;
		}
		if (!scn.load_collada(w_scn_file))
		{
			log_error("Failed to load scene file");
			return;
		}
		auto clr = renderer->new_command<wyc::cmd_clear>();
		clr->color = { 0.0f, 0.0f, 0.0f };
		renderer->enqueue(clr);
		scn.render(renderer);
		renderer->process();
		auto &buffer = render_target->get_color_buffer();
		wyc::CImage image(buffer.get_buffer(), buffer.row_length(), buffer.row(), buffer.pitch());
		if (!image.save(m_img_file))
		{
			log_error("Failed to save image file");
		}
	}
};
