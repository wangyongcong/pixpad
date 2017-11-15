#include "ply.h"
#include <fstream>

namespace wyc
{
	CPlyFile::CPlyFile(const std::string &file_path)
		: m_error(PLY_NO_ERROR)
	{
		_load(file_path);
	}

	CPlyFile::~CPlyFile()
	{
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
		std::string line;
		std::getline(fin, line);
		if (line != "ply") {
			m_error = PLY_INVALID_FILE;
			return false;
		}
		std::string tags[] = {
			"comment",
			"format",
			"element",
			"property",
			"end_header",
		};
		enum PLY_TAG {
			COMMENT = 0,
			FORMAT,
			ELEMENT,
			PROPERTY,
			END_HEADER,
		};
		std::string element_types[] = {
			"vertex",
			"tristrips",
		};
		enum PLY_ELEMENT {
			VERTEX = 0,
			TRISTRIPS,
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
			PlyProperty *prop_lst;
			PlyElement *next;
		};
		auto clean_up = [](PlyElement *elem_lst)
		{
			PlyElement *iter;
			PlyProperty *prop;
			while (elem_lst) {
				iter = elem_lst;
				while (iter->prop_lst) {
					prop = iter->prop_lst;
					iter->prop_lst = prop->next;
					delete prop;
				}
				elem_lst = elem_lst->next;
				delete iter;
			}
		};
		bool is_little_endian = true;
		bool is_binary = true;
		std::string format, version, value;
		PlyElement *elem_lst = nullptr;
		size_t beg, end;
		while (fin) {
			std::getline(fin, line);
			if (0 == line.compare(0, tags[END_HEADER].size(), tags[END_HEADER]))
				break;
			if (0 == line.compare(0, tags[COMMENT].size(), tags[COMMENT]))
				continue;
			end = line.find(' ');
			if (end == std::string::npos)
				continue;
			//log_info(line.c_str());
			if (0 == line.compare(0, end, tags[FORMAT]))
			{
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					//log_error("ply loader: unknown format [%s]", line.c_str());
					return false;
				}
				format = line.substr(beg, end - beg);
				if (format == "ascii")
					is_binary = false;
				else if (format == "is_big_endian")
					is_little_endian = false;
				beg = end + 1;
				version = line.substr(beg);
			}
			else if (0 == line.compare(0, end, tags[ELEMENT]))
			{
				PlyElement *elem = new PlyElement;
				elem->next = elem_lst;
				elem_lst = elem;
				elem->prop_count = 0;
				elem->prop_lst = nullptr;
				// element name
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					//log_error("ply loader: invalid element [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
				elem->name = line.substr(beg, end - beg);
				// element count
				value = line.substr(end + 1);
				try {
					elem->count = std::stoi(value);
				}
				catch (std::invalid_argument) {
					//log_error("ply loader: invalid element [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
			}
			else if (0 == line.compare(0, end, tags[PROPERTY]))
			{
				PlyProperty *prop = new PlyProperty;
				prop->next = elem_lst->prop_lst;
				elem_lst->prop_lst = prop;
				elem_lst->prop_count += 1;
				// property type
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					//log_error("ply loader: invalid property [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
				value = line.substr(beg, end - beg);
				if (value == "float") {
					prop->is_int = false;
					prop->size = 4;
				}
				else if (value == "double") {
					prop->is_int = false;
					prop->size = 8;
				}
				// property name
				prop->name = line.substr(end + 1);
			}
		}
		clean_up(elem_lst);
		return true;
	}

} // namespace wyc
