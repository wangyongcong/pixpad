#pragma once
#include <string>
#include <algorithm>
#include "boost/program_options.hpp"
#include "spw_renderer.h"

#ifdef testbed_EXPORTS
#define EXPORT_API extern "C" _declspec(dllexport)
#define EXPORT_CLASS _declspec(dllexport)
#else
#define EXPORT_API
#define EXPORT_CLASS
#endif

class CTest
{
public:
	static CTest* new_test() {
		return nullptr;
	}

	virtual ~CTest() {
	}

	virtual void init(const boost::program_options::variables_map &args);
	virtual void run() = 0;

	bool has_param(const std::string &name) const {
		auto it = std::find(m_params.begin(), m_params.end(), name);
		return it != m_params.end();
	}
	bool get_param(const std::string &name, std::string &value) const;
	void setup_renderer(unsigned img_w, unsigned img_h, unsigned max_core=0);
	void save_image(const char *name);
	const void* get_color_buf(unsigned &width, unsigned &height, unsigned &pitch_in_pixel) const;
protected:
	std::string m_outfile;
	std::vector<std::string> m_params;
	std::shared_ptr<wyc::CSpwRenderer> m_renderer;
	unsigned m_image_w, m_image_h;
};

#define REGISTER_TEST(name) CTest* create##name() { return new name(); };
#define ENABLE_TEST(name) CTest* create##name();
#define CREATE_TEST(name) create##name
