#include "test.h"
#include "renderer/mesh.h"
#include "mathex/vecmath.h"
#include "mtl_wireframe.h"
#include "mtl_lambert.h"

class CTestLambert : public CTest
{
public:
	virtual void run()
	{
		auto mesh = std::make_shared<wyc::CMesh>();
		mesh->create_sphere(1.0f, 3);

		// setup transform
		wyc::mat4f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 0.1f, 10000);
		wyc::mat4f transform_world;
		wyc::set_translate(transform_world, 0, 0, -3.0f);
		wyc::mat4f proj_from_world = proj * transform_world;
		wyc::mat4f normal_transform;
		normal_transform.makeIdentity();
		wyc::vec4f light_pos_w = { 4.f, 0.f, 2.f, 1.f };
		wyc::vec4f light_pos_view = transform_world * light_pos_w;

		// setup material
		//auto mtl = std::make_shared<CMaterialWireframe>();
		//mtl->set_uniform("proj_from_world", proj_from_world);
		//mtl->set_uniform("line_color", wyc::color4f{ 0, 1, 0, 1 });
		//mtl->set_uniform("fill_color", wyc::color4f{ 0.2f, 0.2f, 0.2f, 1 });

		auto mtl = std::make_shared<CMaterialLambert>();
		mtl->set_uniform("proj_from_world", proj_from_world);
		mtl->set_uniform("view_from_world", transform_world);
		mtl->set_uniform("normal_transform", normal_transform);
		mtl->set_uniform("light_pos_view", wyc::vec3f{ light_pos_view.x, light_pos_view.y, light_pos_view.z });

		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		draw->material = mtl.get();

		m_renderer->enqueue(draw);
		m_renderer->process();
		save_image("lambert.png");
	}
};

REGISTER_TEST(CTestLambert)
