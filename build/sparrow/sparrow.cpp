// sparrow.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <tclap/CmdLine.h>
#include "logger.h"
#include "scene.h"
#include "spw_renderer.h"

#ifdef _DEBUG
	#pragma comment(lib, "libspw_staticd.lib")
	#pragma comment(lib, "Imath-2_2.lib")
	#pragma comment(lib, "IexMath-2_2.lib")
	#pragma comment(lib, "Iex-2_2.lib")
	#pragma comment(lib, "Half.lib")
#else
	#pragma comment(lib, "libspw_static.lib")
#endif

wyc::CLogger *g_log = nullptr;

void do_render(const std::string & scn_file, const std::string & img_file)
{
	unsigned core_count = std::thread::hardware_concurrency();
	std::cout << "max thread count: " << core_count << std::endl;

	auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
	render_target->create(800, 600, wyc::SPR_COLOR_R8G8B8A8);
	auto renderer = std::make_shared<wyc::CSpwRenderer>();
	renderer->set_render_target(render_target);
	wyc::CScene scn;
	std::wstring w_scn_file;
	wyc::CCamera camera;
	if (!wyc::str2wstr(w_scn_file, scn_file))
	{
		error("Invalid file path");
		return;
	}
	if (!scn.load_collada(w_scn_file))
	{
		error("Failed to load scene file");
		return;
	}
	auto cmd = renderer->new_command<wyc::cmd_clear>();
	cmd->color = { 0, 0, 0 };
	renderer->enqueue(cmd);
	scn.render(renderer);
	renderer->process();
	renderer->present();
}

int main(int argc, char *argv[])
{
	try {
		TCLAP::CmdLine cmd("Generate image using Sparrow renderer.", ' ', "0.0.1");
		TCLAP::UnlabeledValueArg<std::string> input_file("input_file", "Input scene file", true, "", "path string");
		cmd.add(input_file);
		TCLAP::UnlabeledValueArg<std::string> output_file("output_file", "Output image file", false, "", "path string");
		cmd.add(output_file);
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
		do_render(scn_file, img_file);
	}
	catch (TCLAP::ArgException &e)
	{
#ifdef error
#undef error
#endif
		std::cerr << "ERROR:" << e.error() << std::endl;
		return 1;
	}
	catch (std::exception &e)
	{
		std::cerr << "ERROR:" << e.what() << std::endl;
		return 2;
	}
    return 0;
}

