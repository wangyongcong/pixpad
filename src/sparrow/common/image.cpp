#include "image.h"
#include <png.h>
#include "log.h"
#include "util.h"

namespace wyc
{
	CImage::CImage()
		: m_data(nullptr)
		, m_width(0)
		, m_height(0)
	{
	}

	CImage::CImage(void * buffer, unsigned width, unsigned height, unsigned pitch)
	{
		size_t sz = height * pitch;
		m_data = new unsigned char[sz];
		memcpy(m_data, buffer, sz);
		m_width = width;
		m_height = height;
		m_pitch = pitch;
	}

	CImage::~CImage()
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}
	}

	CImage::CImage(CImage && other)
		: m_data(other.m_data)
		, m_width(other.m_width)
		, m_height(other.m_height)
	{
		other.m_data = nullptr;
		other.m_width = 0;
		other.m_height = 0;
	}

	CImage & CImage::operator=(CImage && other)
	{
		m_data = other.m_data;
		m_width = other.m_width;
		m_height = other.m_height;
		other.m_data = nullptr;
		other.m_width = 0;
		other.m_height = 0;
		return *this;
	}

	bool CImage::load(const std::wstring &w_file_path)
	{
		std::string file_name;
		if (!wstr2str(file_name, w_file_path)) {
			log_error("load_image: Invalid file path");
			return false;
		}
		return load(file_name);
	}

	bool CImage::save(const std::wstring & w_file_path)
	{
		std::string file_name;
		if (!wstr2str(file_name, w_file_path)) {
			log_error("load_image: Invalid file path");
			return false;
		}
		return save(file_name);
	}

	bool CImage::load(const std::string & file_name)
	{
		auto pos = file_name.rfind(".");
		if (pos == std::string::npos) {
			log_error("load_image: Unknown image format");
			return false;
		}
		std::string ext = file_name.substr(pos + 1);
		if (ext == "png")
		{
			return read_png(file_name);
		}
		log_error("load_image: Unsupported image format [.%s]", ext);
		return false;
	}

	bool CImage::save(const std::string & file_name)
	{
		auto pos = file_name.rfind(".");
		if (pos == std::string::npos) {
			log_error("load_image: Unknown image format");
			return false;
		}
		std::string ext = file_name.substr(pos + 1);
		if (ext == "png")
		{
			return write_png(file_name);
		}
		log_error("load_image: Unsupported image format [.%s]", ext);
		return false;
	}

	bool CImage::read_png(const std::string &file_name)
	{
		png_image image;
		memset(&image, 0, sizeof(png_image));
		image.version = PNG_IMAGE_VERSION;
		if (0 == png_image_begin_read_from_file(&image, file_name.c_str()))
		{
			log_error("load_png: can't read from file [%s]", file_name.c_str());
			return false;
		}
		png_bytep buffer;
		image.format = PNG_FORMAT_RGBA;
		buffer = (png_bytep)new png_byte[PNG_IMAGE_SIZE(image)];
		if (!buffer)
		{
			png_image_free(&image);
			log_error("load_png: memory not enough when reading file [%s]", file_name.c_str());
			return false;
		}
		unsigned row_stride = PNG_IMAGE_ROW_STRIDE(image);
		if (!png_image_finish_read(&image, nullptr, buffer, row_stride, nullptr))
		{
			delete[] buffer;
			png_image_free(&image);
			log_error("load_png: failed to read file [%s]", file_name.c_str());
			return false;
		}
		m_data = buffer;
		m_width = image.width;
		m_height = image.height;
		m_pitch = row_stride;
		return true;
	}

	bool CImage::write_png(const std::string & file_name)
	{
		if (!m_data)
			return false;
		png_image image;
		memset(&image, 0, sizeof(image));
		image.version = PNG_IMAGE_VERSION;
		image.format = PNG_FORMAT_RGBA;
		image.width = m_width;
		image.height = m_height;
		if (!png_image_write_to_file(&image, file_name.c_str(), 0, m_data, m_pitch, nullptr))
		{
			log_error("write_png: failed to write png file [%s]", file_name.c_str());
			return false;
		}
		return true;
	}

} // namespace wyc