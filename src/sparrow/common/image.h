#pragma once
#include <cassert>
#include <string>
#include <vector>
#include <memory>
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
		void clear();
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
			assert(x < int(m_width) && y < int(m_height));
			Imath::C4f color;
			auto row = reinterpret_cast<uint32_t*>(m_data + y * m_pitch);
			Imath::packed2rgb(row[x], color);
			return color;
		}
		// create empty image
		void create_empty(unsigned width, unsigned height);
		// create checker board pattern image
		void create_checkerboard(unsigned size, const Imath::C3f &color1, const Imath::C3f &color2);
		// create mipmap chain
		static bool generate_mipmap(std::vector<std::shared_ptr<CImage>> &image_chain);
	private:
		unsigned char* m_data;
		unsigned m_width;
		unsigned m_height;
		unsigned m_pitch;
		bool m_is_stbi_mem;
	};

} // namespace wyc