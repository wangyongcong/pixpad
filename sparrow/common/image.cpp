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
			error("load_image: Invalid file path");
			return false;
		}
		auto pos = file_name.rfind(".");
		if (pos == std::string::npos) {
			error("load_image: Unknown image format");
			return false;
		}
		std::string ext = file_name.substr(pos + 1);
		if (ext == "png")
		{
			return read_png(file_name);
		}
		error("load_image: Unsupported image format [.%s]", ext);
		return false;
	}

	bool CImage::read_png(const std::string &file_name)
	{
		png_image image;
		memset(&image, 0, sizeof(png_image));
		image.version = PNG_IMAGE_VERSION;
		if (0 == png_image_begin_read_from_file(&image, file_name.c_str()))
		{
			error("load_png: can't read from file [%s]", file_name.c_str());
			return false;
		}
		png_bytep buffer;
		image.format = PNG_FORMAT_RGBA;
		buffer = (png_bytep)new png_byte[PNG_IMAGE_SIZE(image)];
		if (!buffer)
		{
			png_image_free(&image);
			error("load_png: memory not enough when reading file [%s]", file_name.c_str());
			return false;
		}
		if (!png_image_finish_read(&image, nullptr, buffer, PNG_IMAGE_ROW_STRIDE(image), nullptr))
		{
			delete[] buffer;
			png_image_free(&image);
			error("load_png: failed to read file [%s]", file_name.c_str());
			return false;
		}
		m_data = buffer;
		m_width = image.width;
		m_height = image.height;
		return true;
	}

} // namespace wyc
