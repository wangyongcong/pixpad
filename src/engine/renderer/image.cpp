#include "image.h"
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"
#include "stb/stb_image_write.h"
#include "stb/stb_log.h"
#include "common/utility.h"

namespace wyc
{
	CImage::CImage()
		: m_data(nullptr)
		, m_width(0)
		, m_height(0)
		, m_pitch(0)
		, m_is_stbi_mem(true)
	{
	}

	CImage::CImage(const void * buffer, unsigned width, unsigned height, unsigned pitch)
	{
		size_t sz = height * pitch;
		m_data = new unsigned char[sz];
		m_is_stbi_mem = false;
		memcpy(m_data, buffer, sz);
		m_width = width;
		m_height = height;
		m_pitch = pitch;
	}

	CImage::~CImage()
	{
		clear();
	}

	CImage::CImage(CImage && other)
		: m_data(other.m_data)
		, m_width(other.m_width)
		, m_height(other.m_height)
		, m_pitch(other.m_pitch)
		, m_is_stbi_mem(other.m_is_stbi_mem)
	{
		other.m_data = nullptr;
		other.m_width = 0;
		other.m_height = 0;
	}

	CImage & CImage::operator=(CImage && other)
	{
		clear();
		m_data = other.m_data;
		m_is_stbi_mem = other.m_is_stbi_mem;
		m_width = other.m_width;
		m_height = other.m_height;
		m_pitch = other.m_pitch;
		other.m_data = nullptr;
		other.m_width = 0;
		other.m_height = 0;
		other.m_pitch = 0;
		return *this;
	}

	void CImage::clear()
	{
		if (m_data) {
			if (m_is_stbi_mem)
				stbi_image_free(m_data);
			else
				delete[] m_data;
			m_data = nullptr;
			m_width = m_height = m_pitch = 0;
		}
	}

	bool CImage::load(const std::wstring &w_file_path)
	{
		std::string file_name;
		if (!wstr_to_ansi(w_file_path, file_name)) {
			log_error("load_image: Invalid file path");
			return false;
		}
		return load(file_name);
	}

	bool CImage::save(const std::wstring & w_file_path)
	{
		std::string file_name;
		if (!wstr_to_ansi(w_file_path, file_name)) {
			log_error("load_image: Invalid file path");
			return false;
		}
		return save(file_name);
	}

	bool CImage::load(const std::string & file_name)
	{
		clear();
		const int req_channels = 4;
		int w, h, channels;
		auto data = stbi_load(file_name.c_str(), &w, &h, &channels, req_channels);
		if (!data) {
			log_error("load_image: Failed to read image file");
			return false;
		}
		m_data = data;
		m_is_stbi_mem = true;
		m_width = w;
		m_height = h;
		m_pitch = m_width * req_channels;
		return true;
	}

	bool CImage::save(const std::string & file_name)
	{
		if (!m_data)
			return false;
		auto pos = file_name.rfind(".");
		if (pos == std::string::npos) {
			log_error("save_image: Unknown image format");
			return false;
		}
		std::string ext = file_name.substr(pos + 1);
		int ret = 0;
		if (ext == "png")
		{
			ret = stbi_write_png(file_name.c_str(), m_width, m_height, 4, m_data, 0);
		}
		else {
			log_error("save_image: Unsupported image format [.%s]", ext);
			return false;
		}
		if (!ret) {
			log_error("save_image: Failed to write image file [%s]", file_name.c_str());
			return false;
		}
		return true;
	}

	void CImage::create_empty(unsigned width, unsigned height)
	{
		clear();
		unsigned pitch = width * 4;
		unsigned size = pitch * height;
		m_data = new unsigned char[size];
		m_is_stbi_mem = false;
		m_width = width;
		m_height = height;
		m_pitch = pitch;
	}

	void CImage::create_checkerboard(unsigned size, const color3f &color1, const color3f &color2)
	{
		clear();
		size = wyc::minimal_power2(size);
		m_width = m_height = size;
		m_pitch = size * sizeof(uint32_t);
		m_data = new unsigned char[m_pitch * size];
		m_is_stbi_mem = false;
		int half = size >> 1;
		auto p1 = (uint32_t*)m_data;
		auto p2 = p1 + half;
		uint32_t c1 = Imath::rgb2packed(color1);
		uint32_t c2 = Imath::rgb2packed(color2);
		for (auto k = 0; k < 2; ++k) {
			for (auto j = 0; j < half; ++j) {
				for (auto i = 0; i < half; ++i) {
					p1[i] = c1;
					p2[i] = c2;
				}
				p1 += size;
				p2 += size;
			}
			std::swap(c1, c2);
		}
	}

	bool CImage::generate_mipmap(std::vector<std::shared_ptr<CImage>>& image_chain)
	{
		if (image_chain.empty())
			return false;
		auto img0 = image_chain[0];
		auto w = img0->width();
		auto h = img0->height();
		if ((w & (w - 1)) || (h & (h - 1)))
			// size should be power of 2
			return false;
		image_chain.resize(1);
		while (w > 1 && h > 1)
		{
			w >>= 1;
			h >>= 1;
			auto new_img = std::make_shared<CImage>();
			new_img->create_empty(w, h);
			int ret = stbir_resize_uint8(img0->m_data, img0->m_width, img0->m_height, img0->m_pitch,
				new_img->m_data, new_img->m_width, new_img->m_height, new_img->m_pitch, 4);
			if (ret != 1)
				return false;
			image_chain.push_back(new_img);
		}
		return true;
	}

} // namespace wyc
