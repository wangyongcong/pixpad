#pragma once
#include "test.h"
#include "vecmath.h"
#include "mesh.h"
#include "mtl_diffuse.h"
#include <boost/filesystem.hpp>

void create_floor(wyc::CMesh* mesh, float r, float w=1)
{
	using namespace wyc;
	struct Vertex {
		Imath::V3f pos;
		Imath::V4f color;
		Imath::V2f uv;
	};
	VertexAttrib attrib_array[] = {
		{ATTR_POSITION, 3, offsetof(Vertex, pos)},
		{ATTR_COLOR, 4, offsetof(Vertex, color)},
		{ATTR_UV0, 2, offsetof(Vertex, uv)},
	};
	Vertex verts[4] = {
		{ {-r, 0,  r },{ 1, 1, 1, 1 },{ 0, 0 } },
		{ { r, 0,  r },{ 1, 1, 1, 1 },{ w, 0 } },
		{ { r, 0, -r },{ 1, 1, 1, 1 },{ w, w } },
		{ {-r, 0, -r },{ 1, 1, 1, 1 },{ 0, w } },
	};
	mesh->set_vertices(attrib_array, 3, verts, 4);
	mesh->set_indices({
		0, 1, 2, 0, 2, 3,
	});
}

bool generate_mipmap(const std::string &image_file)
{
	auto base_img = std::make_shared<wyc::CImage>();
	boost::filesystem::path file_path(image_file);
	if (!base_img->load(image_file)) {
		return false;
	}
	std::vector<std::shared_ptr<wyc::CImage>> mipmap_chain;
	mipmap_chain.push_back(base_img);
	if (wyc::CImage::generate_mipmap(mipmap_chain))
	{
		log_info("create mipmap OK");
		int lod = 0;
		boost::filesystem::path base_name = file_path.stem(), file_path;
		boost::filesystem::create_directory(base_name);	
		std::string ext = ".png";
		for (auto img : mipmap_chain)
		{

			file_path = base_name / std::to_string(lod);
			file_path += ext;
			img->save(file_path.string());
			lod += 1;
		}
	}
	else {
		log_error("fail to create mipmap!");
	}
	return true;
}

class CTestMipmap : public CTest
{
public:
	static CTest* create() {
		return new CTestMipmap();
	}

	virtual void run() {
		//generate_mipmap("res/lenna.png");
		//return;

		auto mesh = std::make_shared<wyc::CMesh>();
		create_floor(mesh.get(), 16, 32);

		// setup transform
		Imath::M44f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 1000);
		Imath::M44f rx_world, ry_world, transform_world;
		wyc::set_rotate_x(rx_world, wyc::deg2rad(10));
		wyc::set_rotate_y(ry_world, wyc::deg2rad(20));
		wyc::set_translate(transform_world, 0, -1.5, -10);
		Imath::M44f proj_from_world;
		proj_from_world = proj * transform_world * rx_world * ry_world;

		// setup material
		auto mtl = std::make_shared<wyc::CMaterialDiffuse>();
		mtl->set_uniform("proj_from_world", proj_from_world);
		// sampler
		auto diffuse_img = std::make_shared<wyc::CImage>();
		diffuse_img->create_checkerboard(16, {1, 1, 1}, {0, 0, 0});
		auto sampler = std::make_shared<wyc::CSpwSampler>(diffuse_img.get());
		mtl->set_uniform("diffuse", (wyc::CSampler*)sampler.get());

		// draw 
		auto draw = m_renderer->new_command<wyc::cmd_draw_mesh>();
		draw->mesh = mesh.get();
		draw->material = mtl.get();
		m_renderer->enqueue(draw);
		m_renderer->process();

		// save result
		save_image("mipmap.png");

	}
};