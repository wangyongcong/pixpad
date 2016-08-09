// sparrow.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <tclap/CmdLine.h>
#include "logger.h"
#include "scene.h"
#include "spw_renderer.h"
#include "image.h"
#include "mtl_flat_color.h"
#include "spw_pipeline_wireframe.h"

#ifdef _DEBUG
	#pragma comment(lib, "libspw_staticd.lib")
	#pragma comment(lib, "Imath-2_2.lib")
	#pragma comment(lib, "IexMath-2_2.lib")
	#pragma comment(lib, "Iex-2_2.lib")
	#pragma comment(lib, "Half.lib")
	#pragma comment(lib, "libpng16_staticd.lib")
	#pragma comment(lib, "zlibstaticd.lib")
#else
	#pragma comment(lib, "libspw_static.lib")
#endif

wyc::CLogger *g_log = nullptr;

void do_render(const std::string & scn_file, const std::string & img_file, bool is_wireframe)
{
	unsigned core_count = std::thread::hardware_concurrency();
	log_debug("max thread count: %d", core_count);
	auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
	render_target->create(960, 540, wyc::SPR_COLOR_R8G8B8A8);
	auto renderer = std::make_shared<wyc::CSpwRenderer>();
	renderer->set_render_target(render_target);
	std::shared_ptr<wyc::CSpwPipeline> pipeline;
	if (is_wireframe)
		pipeline = std::make_shared<wyc::CSpwPipelineWireFrame>();
	else
		pipeline = std::make_shared<wyc::CSpwPipeline>();
	renderer->set_pipeline(pipeline);
	wyc::CScene scn;
	std::wstring w_scn_file;
	wyc::CCamera camera;
	if (!wyc::str2wstr(w_scn_file, scn_file))
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
	if (!image.save(img_file))
	{
		log_error("Failed to save image file");
	}
}

int main(int argc, char *argv[])
{
	try {
		TCLAP::CmdLine cmd("Generate image using Sparrow renderer.", ' ', "0.0.1");
		TCLAP::UnlabeledValueArg<std::string> input_file("input_file", "Input scene file", true, "", "path string");
		cmd.add(input_file);
		TCLAP::UnlabeledValueArg<std::string> output_file("output_file", "Output image file", false, "", "path string");
		cmd.add(output_file);
		TCLAP::SwitchArg is_wireframe("w", "wireframe", "Enable wireframe mode", cmd);
		cmd.parse(argc, argv);
		std::string scn_file = input_file.getValue();
		std::string img_file = output_file.getValue();
		if (img_file.empty())
		{
			size_t end = scn_file.rfind(".");
			if (end == std::string::npos)
				img_file = scn_file + ".png";
			else {
				std::string base_name = scn_file.substr(0, end);
				img_file = base_name + ".png";
			}
		}
		g_log = new wyc::CDebugLogger();
		do_render(scn_file, img_file, is_wireframe.getValue());
	}
	catch (TCLAP::ArgException &e)
	{
		log_error("ERROR: %s", e.error().c_str());
		return 1;
	}
	catch (std::exception &e)
	{
		log_error("ERROR: %s", e.what());
		return 2;
	}
	log_info("finish and exit");
    return 0;
}

