#include "test.h"
#include "image.h"

void CTest::init(const boost::program_options::variables_map &args) {
	if (args.count("out")) {
		m_outfile = args["out"].as<std::string>();
	}
	if (args.count("param")) {
		m_params = args["param"].as<std::vector<std::string>>();
	}
	m_image_w = std::min<int>(2048, args["width"].as<unsigned>());
	m_image_h = std::min<int>(2048, args["height"].as<unsigned>());
	setup_renderer(m_image_w, m_image_h);
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

void CTest::setup_renderer(unsigned img_w, unsigned img_h)
{
	m_renderer = std::make_shared<wyc::CSpwRenderer>();
	// create render target
	auto render_target = std::make_shared<wyc::CSpwRenderTarget>();
	render_target->create(img_w, img_h, wyc::SPR_COLOR_R8G8B8A8 | wyc::SPR_DEPTH_32);
	m_renderer->set_render_target(render_target);
	// create pipeline
	auto pipeline = std::make_shared<wyc::CSpwPipeline>();
	pipeline->setup();
	m_renderer->set_pipeline(pipeline);
	// clear
	auto clr = m_renderer->new_command<wyc::cmd_clear>();
	clr->color = { 0.0f, 0.0f, 0.0f };
	m_renderer->enqueue(clr);
}

void CTest::save_image(const char *name)
{
	auto render_target = std::dynamic_pointer_cast<wyc::CSpwRenderTarget>(m_renderer->get_render_target());
	auto &buffer = render_target->get_color_buffer();
	wyc::CImage image(buffer.get_buffer(), buffer.row_length(), buffer.row(), buffer.pitch());
	if (m_outfile.empty()) {
		m_outfile = name;
	}
	if (!image.save(m_outfile))
	{
		log_error("Failed to save image file");
	}
}
