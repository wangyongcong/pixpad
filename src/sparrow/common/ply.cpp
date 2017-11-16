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
		while (fin && value != end) {
			std::getline(fin, value);
			out << std::endl << value;
		}
	}

	CPlyFile::CPlyFile(const std::string &file_path)
		: m_error(PLY_NO_ERROR)
		, m_elements(nullptr)
	{
		_load(file_path);
	}

	CPlyFile::~CPlyFile()
	{
		_clear();
	}

	const char * CPlyFile::get_error_desc() const
	{
		static const char *s_ply_error_desc[] = {
			"no error",
			"file not found",
			"invalid ply file",
			"unknown format",
			"unknown element",
			"unknown property",
			"invalid property",
			"not support ascii format",
			"not support big endian format",
		};
		return s_ply_error_desc[m_error];
	}

	void CPlyFile::detail(std::ostream & out) const
	{
		if (!m_elements)
		{
			out << "No elements" << std::endl;
			return;
		}
		for (auto elem = m_elements; elem; elem = elem->next)
		{
			out << elem->name << "(" << elem->count << ")" << std::endl;
			for (auto prop = elem->properties; prop; prop = prop->next)
			{
				out << "    " << prop->name << "(";
				if (prop->type == PLY_LIST)
					out << "L" << (prop->size >> 16) << "/" << (prop->size & 0xFFFF);
				else if(prop->type == PLY_INTEGER)
					out << "i" << prop->size;
				else
					out << "f" << prop->size;
				out << ")" << std::endl;
			}
		}
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
	static std::pair<PLY_PROPERTY_TYPE, uint8_t> ply_property_type(const std::string &type)
	{
		if (type == "char" || type == "uchar") {
			return std::make_pair(PLY_INTEGER, 1);
		}
		else if (type == "short" || type == "ushort") {
			return std::make_pair(PLY_INTEGER, 2);
		}
		else if (type == "int" || type == "uint") {
			return std::make_pair(PLY_INTEGER, 4);
		}
		else if (type == "float") {
			return std::make_pair(PLY_FLOAT, 4);
		}
		else if (type == "double") {
			return std::make_pair(PLY_FLOAT, 8);
		}
		return std::make_pair(PLY_UNKNOWN_TYPE, 0);
	}

	bool CPlyFile::_load(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			m_error = PLY_FILE_NOT_FOUND;
			return false;
		}
		std::stringstream line;
		std::string value, type;
		std::getline(fin, value);
		if (value != "ply") {
			m_error = PLY_INVALID_FILE;
			return false;
		}
		unsigned count;
		constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
		bool is_little_endian = true;
		bool is_binary = true;
		if (m_elements)
			_clear();
		PlyElement **tail = &m_elements;
		PlyProperty **prop_tail = nullptr;
		while (fin) {
			std::getline(fin, value);
			if (value.empty() || value == PLY_TAGS[END_HEADER])
				break;
			line.clear();
			line.str(value);
			// read tag
			line >> value;
			if (value == PLY_TAGS[COMMENT]) {
				// skip comment
			}
			else if (value == PLY_TAGS[FORMAT])
			{
				// format & version
				line >> value >> type;
				if (value == "ascii")
					is_binary = false;
				else if (value == "binary_big_endian")
				{
					is_binary = true;
					is_little_endian = false;
				}
				else if (value == "binary_little_endian") {
					is_binary = true;
					is_little_endian = true;
				}
				else {
					m_error = PLY_UNKNOWN_FORMAT;
					return false;
				}
			}
			else if (value == PLY_TAGS[ELEMENT])
			{
				// element name and count
				value = "";
				count = 0;
				line >> value >> count;
				if (line.fail()) 
					continue;
				if (!value.empty() && count) {
					PlyElement *elem = new PlyElement;
					elem->name = value;
					elem->count = count;
					elem->prop_count = 0;
					elem->properties = nullptr;
					elem->next = nullptr;
					*tail = elem;
					tail = &elem->next;
					prop_tail = &elem->properties;
				}
			}
			else if (value == PLY_TAGS[PROPERTY])
			{
				// property type
				line >> value;
				if (line.fail() || value.empty())
					continue;
				PlyProperty *prop = new PlyProperty;
				//prop->name = value;
				prop->next = nullptr;
				*prop_tail = prop;
				prop_tail = &prop->next;
				if (value == "list") {
					prop->type = PLY_LIST;
					line >> value;
					auto t1 = ply_property_type(value);
					line >> value;
					auto t2 = ply_property_type(value);
					if (line.fail() || t1.first != PLY_INTEGER) {
						m_error = PLY_INVALID_PROPERTY;
						return false;
					}
					prop->size = (t1.second << 16) | t2.second;
				}
				else {
					auto t = ply_property_type(value);
					prop->type = t.first;
					prop->size = t.second;
				}
				// property name
				line >> prop->name;
			}
		}
		if (is_binary) {
			if (!is_little_endian)
			{
				m_error = PLY_NOT_SUPPORT_BID_ENDIAN;
				return false;
			}
			return _read_binay(fin);
		}
		m_error = PLY_NOT_SUPPORT_ASCII;
		return false;
	}

	bool CPlyFile::_read_binay(std::ifstream & fin)
	{
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
		m_elements = nullptr;
	}

} // namespace wyc
