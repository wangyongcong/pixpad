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
		wyc::vec3f pos;
		wyc::vec4f color;
		wyc::vec2f uv;
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

class CMaterialDiffuseMipmap : public CMaterialDiffuse
{
	UNIFORM_MAP{
		UNIFORM_SLOT(wyc::mat4f, proj_from_world)
		UNIFORM_SLOT(wyc::CSampler*, diffuse)
		UNIFORM_SLOT(unsigned, texture_size)
		UNIFORM_MAP_END
	};

public:
	CMaterialDiffuseMipmap()
		: CMaterialDiffuse()
		, texture_size(0)
	{
	}

	virtual bool fragment_shader(const void *frag_in, wyc::color4f &frag_color, wyc::CShaderContext *ctx) const override
	{
		auto in = reinterpret_cast<const VertexOut*>(frag_in);
		wyc::color4f diffuse_color;
		auto duvdx = ctx->ddx(&VertexOut::uv);
		auto duvdy = ctx->ddy(&VertexOut::uv);
		float r1 = std::sqrtf(duvdx ^ duvdx);
		float r2 = std::sqrtf(duvdy ^ duvdy);
		union {
			float f;
			unsigned i;
		} j;
		j.f = std::max(r1, r2) * texture_size;
		int e = (j.i & 0x7f800000) >> 23;
		if (e > 127) {
			e -= 127;
			int m = j.i & 0x7fffff;
			float f = float(m) / (1 << 23);
			wyc::color4f c1, c2;
			diffuse->sample2d(in->uv, e, c1);
			diffuse->sample2d(in->uv, e-1, c2);
			diffuse_color = c1 * f + c2 * (1 - f);
		}
		else
			diffuse->sample2d(in->uv, diffuse_color);
		frag_color = diffuse_color * in->color;
		return true;
	}
protected:
	unsigned texture_size;
};

class CTestMipmap : public CTest
{
public:
	virtual void run() {
		//generate_mipmap("res/checkerboard.png");
		//return;

		auto mesh = std::make_shared<wyc::CMesh>();
		create_floor(mesh.get(), 16, 32);

		// setup transform
		wyc::mat4f proj;
		wyc::set_perspective(proj, 45, float(m_image_w) / m_image_h, 1, 1000);
		wyc::mat4f rx_world, ry_world, transform_world;
		wyc::set_rotate_x(rx_world, wyc::deg2rad(4));
		wyc::set_rotate_y(ry_world, wyc::deg2rad(16));
		wyc::set_translate(transform_world, 0, -0.2f, -10);
		wyc::mat4f proj_from_world;
		proj_from_world = proj * transform_world * rx_world * ry_world;

		// setup material
		auto mtl = std::make_shared<CMaterialDiffuseMipmap>();
		//auto mtl = std::make_shared<CMaterialDiffuse>();
		mtl->set_uniform("proj_from_world", proj_from_world);
		// sampler
		auto diffuse_img = std::make_shared<wyc::CImage>();
		diffuse_img->load("res/checkerboard.png");
		//diffuse_img->create_checkerboard(64, {1, 1, 1}, {0, 0, 0});
		std::vector<decltype(diffuse_img)> mipmap_images = { diffuse_img };
		if (!diffuse_img->generate_mipmap(mipmap_images)) {
			log_error("create mipmap error");
			return;
		}
		auto sampler = std::make_shared<wyc::CSpwMipmapSampler>(mipmap_images);
		mtl->set_uniform("diffuse", (wyc::CSampler*)sampler.get());
		mtl->set_uniform("texture_size", diffuse_img->width());

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
REGISTER_TEST(CTestMipmap)