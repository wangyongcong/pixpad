#pragma once
#include <string>
#include <OpenEXR/ImathColorAlgo.h>

namespace wyc
{
	
	class CImage
	{
	public:
		CImage();
		CImage(void *buffer, unsigned width, unsigned height, unsigned pitch);
		~CImage();
		CImage(const CImage&) = delete;
		CImage& operator = (const CImage&) = delete;
		CImage(CImage &&other);
		CImage& operator = (CImage&& other);
		bool load(const std::wstring &file_name);
		bool save(const std::wstring &file_name);
		bool load(const std::string &file_name);
		bool save(const std::string &file_name);
		inline unsigned width() const {
			return m_width;
		}
		inline unsigned height() const {
			return m_height;
		}
		inline Imath::C4f get_color(int x, int y) const {
			uint32_t *pixels = reinterpret_cast<uint32_t*>(m_data);
			Imath::PackedColor packed = pixels[y * m_width + x];
			Imath::C4f color;
			Imath::packed2rgb(packed, color);
			return color;
		}
	private:
		unsigned char* m_data;
		unsigned m_width;
		unsigned m_height;
		unsigned m_pitch;

		bool read_png(const std::string &file_name);
		bool write_png(const std::string &file_name);
	};

} // namespace wyc