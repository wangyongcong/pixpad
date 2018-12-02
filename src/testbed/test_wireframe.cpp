#include "test.h"
#include "mesh.h"
#include "mtl_wireframe.h"

using namespace wyc;

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
		auto pos = ply_file.rfind(".");
		if (pos != std::string::npos) {
			if (ply_file.substr(pos + 1) != "ply") {
				log_error("model file not support");
				return;
			}
		}

		auto mesh = std::make_shared<CMesh>();
		if (!mesh->load_ply(ply_file)) {
			return;
		}

		// setup transform
		wyc::mat4f proj;
		set_perspective(proj, 45, float(m_image_w) / m_image_h, 0.1f, 10000);
		wyc::mat4f rx_world, ry_world, transform_world;
		set_rotate_y(ry_world, deg2rad(45));
		set_rotate_x(rx_world, deg2rad(15));
		wyc::mat4f proj_from_world;

		auto draw = m_renderer->new_command<cmd_draw_mesh>();
		draw->mesh = mesh.get();
		auto mtl = std::make_shared<CMaterialWireframe>();
		// icosahedron
		set_translate(transform_world, 0, 0, -3.2f);
		proj_from_world = proj * transform_world * rx_world * ry_world;
		// torus
		//set_translate(transform_world, 0, 0, -3.6f);
		//proj_from_world = proj * transform_world * rx_world * ry_world;
		// sofa
		//set_translate(transform_world, 0, -500, -2400);
		//proj_from_world = proj * transform_world;
		mtl->set_uniform("proj_from_world", proj_from_world);
		mtl->set_uniform("line_color", wyc::color4f{ 0, 1, 0, 1 });
		mtl->set_uniform("fill_color", wyc::color4f{ 0.2f, 0.2f, 0.2f, 1 });
		draw->material = mtl.get();
		m_renderer->enqueue(draw);

		m_renderer->process();
		pos = ply_file.rfind("/");
		std::string out;
		if (pos != std::string::npos)
			out = ply_file.substr(pos + 1);
		else
			out = ply_file;
		out.replace(out.size() - 4, 4, ".png");
		save_image(out.c_str());
	}

private:
};

REGISTER_TEST(CTestWireframe)
