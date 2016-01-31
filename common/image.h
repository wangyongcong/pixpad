#pragma once
#include <string>

namespace wyc
{
	
	class CImage
	{
	public:
		CImage();
		~CImage();
		CImage(const CImage&) = delete;
		CImage& operator = (const CImage&) = delete;
		CImage(CImage &&other);
		CImage& operator = (CImage&& other);

		bool load(const std::wstring &file_name);
	private:
		unsigned char* m_data;
		unsigned m_width;
		unsigned m_height;

		bool read_png(const std::string &file_name);
	};

} // namespace wyc