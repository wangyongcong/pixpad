#include "test.h"
#include "image.h"

void CTest::init(const boost::program_options::variables_map &args) {
	if (args.count("out")) {
		m_outfile = args["out"].as<std::string>();
	}
	if (args.count("param")) {
		m_params = args["param"].as<std::vector<std::string>>();
	}
	auto max_core = args["core"].as<unsigned>();
	m_image_w = std::min<int>(2048, args["width"].as<unsigned>());
	m_image_h = std::min<int>(2048, args["height"].as<unsigned>());
	setup_renderer(m_image_w, m_image_h, max_core);
}

bool CTest::get_param(const std::string &name, std::string &value) const {
	if (name.empty()) {
		return false;
	}
	size_t sz = name.size();
	for (auto &v : m_params) {
		if (v.size() < sz || 0 != strncmp(v.c_str(), name.c_str(), sz))
			continue;
		if (v.size() == sz)
			return true;
		else if (v[sz] == '=')
		{
			value = v.substr(sz + 1);
			return true;
		}
	}
	return false;
}

void CTest::setup_renderer(unsigned img_w, unsigned img_h, unsigned max_core)
{
	m_renderer = std::make_shared<wyc::CSpwRenderer>();
	// create render target
	auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
	render_target->create(img_w, img_h, wyc::SPW_COLOR_RGBA_F32 | wyc::SPW_DEPTH_32);
	m_renderer->set_render_target(render_target);
	// create pipeline
	auto pipeline = std::make_shared<wyc::CSpwPipeline>();
	pipeline->setup(max_core);
	m_renderer->set_pipeline(pipeline);
	// create LDR image buffer
	m_ldr_image.storage(img_w, img_h, 4);
	// clear
	auto clr = m_renderer->new_command<wyc::cmd_clear>();
	m_renderer->enqueue(clr);
}

void CTest::save_image(const char *name)
{
	unsigned width, height, pitch_in_pixel;
	auto buf = get_color_buf(width, height, pitch_in_pixel);
	wyc::CImage image(buf, width, height, m_ldr_image.pitch());
	if (m_outfile.empty()) {
		m_outfile = name;
	}
	if (!image.save(m_outfile))
	{
		log_error("Failed to save image file");
	}
}

const void * CTest::get_color_buf(unsigned & width, unsigned & height, unsigned & pitch_in_pixel) const
{
	auto render_target = std::dynamic_pointer_cast<wyc::CSpwRenderTarget>(m_renderer->get_render_target());
	auto &buffer = render_target->get_color_buffer();
	width = buffer.row_length();
	height = buffer.row();
	assert(m_ldr_image.row_length() == width && m_ldr_image.row() == height);
	// NOTICE: we assume that render target is 4-bytes RGBA format
	pitch_in_pixel = m_ldr_image.pitch() / 4;
	// linear space to sRGB space
	constexpr float gamma = 1 / 2.2f;
	for (unsigned y = 0; y < height; ++y) {
		auto iter = (const wyc::color4f*)buffer.get_line(y);
		auto end = iter + width;
		auto out = (uint32_t*)m_ldr_image.get_line(y);
		for (; iter != end; ++iter) {
			wyc::color4f c = {
				std::pow(iter->r, gamma),
				std::pow(iter->g, gamma),
				std::pow(iter->b, gamma),
				iter->a
			};
			*out++ = Imath::rgb2packed(c);
		}
	}
	return m_ldr_image.get_buffer();
}
