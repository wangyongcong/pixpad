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

	struct PlyProperty
	{
		std::string name;
		uint8_t size;
		bool is_int;
		PlyProperty *next;
	};

	struct PlyElement
	{
		std::string name;
		unsigned count;
		unsigned prop_count;
		PlyProperty *properties;
		PlyElement *next;
	};

	class CPlyFile
	{
	public:
		static void read_header(std::ostream &out, const std::string &file_path);

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
		void _clear();
		PLY_ERROR m_error;
		PlyElement *m_elements;
	};

} // namespace wyc