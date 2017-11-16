#include "ply.h"
#include <fstream>
#include <sstream>
#include <limits>

namespace wyc
{
	const char* PLY_TAGS[] = {
		"comment",
		"format",
		"element",
		"property",
		"end_header",
	};

	enum PLY_TAG_TYPE {
		COMMENT = 0,
		FORMAT,
		ELEMENT,
		PROPERTY,
		END_HEADER,
	};

	void CPlyFile::read_header(std::ostream &out, const std::string &path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
			return;
		const char *end = PLY_TAGS[END_HEADER];
		std::string value;
		out << std::endl;
		while (fin && value != end) {
			std::getline(fin, value);
			out << value << std::endl;
		}
	}

	CPlyFile::CPlyFile(const std::string &file_path)
		: m_error(PLY_NO_ERROR)
		, m_elements(nullptr)
	{
		if (!_load(file_path))
			_clear();
	}

	CPlyFile::~CPlyFile()
	{
		_clear();
	}

	const char * CPlyFile::get_error_desc() const
	{
		static const char *s_ply_error_desc[] = {
			nullptr,
			"file not found",
			"invalid ply file",
			"unknown format",
			"unknown element",
			"unknown property",
		};
		return s_ply_error_desc[m_error];
	}

	/*
	# name        type        number of bytes
	# ---------------------------------------
	# char       character                 1
	# uchar      unsigned character        1
	# short      short integer             2
	# ushort     unsigned short integer    2
	# int        integer                   4
	# uint       unsigned integer          4
	# float      single-precision float    4
	# double     double-precision float    8
	*/
	bool CPlyFile::_load(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			m_error = PLY_FILE_NOT_FOUND;
			return false;
		}
		std::stringstream line;
		std::string value, version;
		std::getline(fin, value);
		if (value != "ply") {
			m_error = PLY_INVALID_FILE;
			return false;
		}
		std::string element_types[] = {
			"vertex",
			"tristrips",
		};
		enum PLY_ELEMENT {
			VERTEX = 0,
			TRISTRIPS,
		};
		bool is_little_endian = true;
		bool is_binary = true;
		unsigned count;
		constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
		size_t beg, end;
		while (fin) {
			std::getline(fin, value);
			if (value.empty() || value == PLY_TAGS[END_HEADER])
				break;
			line.str(value);
			// read tag
			line >> value;
			if (value == PLY_TAGS[COMMENT]) {
				// skip comment
			}
			else if (value == PLY_TAGS[FORMAT])
			{
				line >> value >> version;
				is_binary = value == "ascii";
				is_little_endian = value == "is_big_endian";
			}
			else if (value == PLY_TAGS[ELEMENT])
			{
				// element name and count
				value = "";
				count = 0;
				line >> value >> count;
				if (line.fail()) {
					line.clear(std::ios_base::failbit);
					continue;
				}
				if (!value.empty() && count) {
					PlyElement *elem = new PlyElement;
					elem->name = value;
					elem->count = count;
					elem->prop_count = 0;
					elem->properties = nullptr;
					elem->next = m_elements;
					m_elements = elem;
				}
			}
			else if (value == PLY_TAGS[PROPERTY])
			{
				// property type
				line >> value;
				// property name
				fin >> value;
				PlyProperty *prop = new PlyProperty;
				prop->name = value;
				prop->next = m_elements->properties;
				m_elements->properties = prop;
				m_elements->prop_count += 1;
			}
		}
		return true;
	}

	void CPlyFile::_clear()
	{
		for (auto elem = m_elements; elem;)
		{
			for (auto prop = elem->properties; prop;) {
				auto next_prop = prop->next;
				delete prop;
				prop = next_prop;
			}
			auto next_elem = elem->next;
			delete elem;
			elem = next_elem;
		}
	}

} // namespace wyc
