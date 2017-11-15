#pragma once
#include <string>

namespace wyc
{
	enum PLY_ERROR {
		PLY_NO_ERROR = 0,
		PLY_FILE_NOT_FOUND,
		PLY_INVALID_FILE,
		PLY_UNKNOWN_FORMAT,
		PLY_UNKNOWN_ELEMENT,
		PLY_UNKNOWN_PROPERTY,
	};

	class CPlyFile
	{
	public:
		CPlyFile(const std::string &file_path);
		~CPlyFile();
		inline operator bool() const {
			return m_error == PLY_NO_ERROR;
		}
		inline PLY_ERROR get_error() const {
			return m_error;
		}
		const char* get_error_desc() const;

	private:
		bool _load(const std::string &file_path);
		PLY_ERROR m_error;
	};

} // namespace wyc