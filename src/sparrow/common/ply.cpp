#include "ply.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <vector>
#include <unordered_map>
#include "util.h"

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
		else if (0 == type.compare(0, 3, "int"))
		{
			if (type.size() > 3)
			{
				std::string sub = type.substr(3);
				int sz = 0;
				try {
					sz = std::stol(sub);
				}
				catch (std::invalid_argument) {
					return std::make_pair(PLY_NULL, 0);
				}
				sz = (sz + 7) / 8;
				return std::make_pair(PLY_INTEGER, sz);
			}
			else {
				return std::make_pair(PLY_INTEGER, 4);
			}
		}
		else if (0 == type.compare(0, 4, "uint")) {
			if (type.size() > 4) {
				std::string sub = type.substr(4);
				int sz = 0;
				try {
					sz = std::stol(sub);
				}
				catch (std::invalid_argument) {
					return std::make_pair(PLY_NULL, 0);
				}
				sz = (sz + 7) / 8;
				return std::make_pair(PLY_INTEGER, sz);
			}
			else {
				return std::make_pair(PLY_INTEGER, 4);
			}
		}
		else if (0 == type.compare(0, 5, "float")) {
			if (type.size() > 5)
			{
				std::string sub = type.substr(5);
				int sz = 0;
				try {
					sz = std::stol(sub);
				}
				catch (std::invalid_argument) {
					return std::make_pair(PLY_NULL, 0);
				}
				sz = (sz + 7) / 8;
				return std::make_pair(PLY_FLOAT, sz);
			}
			else {
				return std::make_pair(PLY_FLOAT, 4);
			}
		}
		else if (type == "double") {
			return std::make_pair(PLY_FLOAT, 8);
		}
		return std::make_pair(PLY_NULL, 0);
	}

	bool CPlyFile::_load(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::binary);
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
		PlyElement *cur_elem = nullptr;
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
					cur_elem = new PlyElement;
					cur_elem->name = value;
					cur_elem->count = count;
					cur_elem->size = 0;
					cur_elem->is_variant = false;
					cur_elem->properties = nullptr;
					cur_elem->next = nullptr;
					*tail = cur_elem;
					tail = &cur_elem->next;
					prop_tail = &cur_elem->properties;
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
					prop->size = (t1.second << 24) | (t1.second << 16) | (t2.second << 8) | t2.first;
					cur_elem->is_variant = true;
				}
				else {
					auto t = ply_property_type(value);
					prop->type = t.first;
					prop->size = t.second;
					cur_elem->size += prop->size;
				}
				// property name
				line >> prop->name;
			}
		}
		auto data_start = fin.tellg();
		if (!is_binary)
			return _read_ascii(fin);
		else if (is_little_endian)
			return _read_binary_le(fin, data_start);
		else
			return _read_binary_be(fin);
	}

	class CPlyIgnoreSize: public IPlyReader
	{
	public:
		CPlyIgnoreSize(std::streampos sz)
			: m_size(sz)
		{

		}
		virtual void operator() (std::istream &in) override {
			in.ignore(m_size);
		}
	private:
		std::streampos m_size;
	};

	class CPlyIgnoreList : public IPlyReader
	{
	public:
		CPlyIgnoreList(unsigned bytes_for_length, unsigned element_size)
			: m_bytes_for_length(bytes_for_length)
			, m_element_size(element_size)
		{
		}

		virtual void operator() (std::istream &in) override {
			uint64_t length = 0;
			in.read((char*)&length, m_bytes_for_length);
			in.ignore(length * m_element_size);
		}
	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
	};

	class CPlyReadFloat : public IPlyReader
	{
	public:
		CPlyReadFloat(std::streampos sz)
			: m_size(sz)
		{

		}
		virtual void operator() (std::istream &in) override {
			union {
				float f32;
				double f64;
			} var;
			in.read((char*)&var, m_size);
		}
	private:
		std::streampos m_size;
	};


	void CPlyFile::_read_vertex(std::istream & in, PlyElement *elem)
	{
		uint8_t comp_pos = 0;
		uint8_t comp_color = 0;
		uint8_t comp_normal = 0;
		std::unordered_map<std::string, uint8_t*> interest_props = {
			{ "x", &comp_pos },
			{ "y", &comp_pos },
			{ "z", &comp_pos },
		};
		unsigned skip = 0;
		std::vector<IPlyReader*> readers;
		for (auto prop = elem->properties; prop; prop = prop->next)
		{
			auto iter = interest_props.find(prop->name);
			if (iter != interest_props.end()) {
				*(iter->second) += 1;
				if (skip > 0) {
					readers.push_back(new CPlyIgnoreSize(skip));
					skip = 0;
				}
			}
			else {
				skip += prop->size;
			}
		}
		// clear readers
		for (auto r : readers)
			delete r;
	}

	bool CPlyFile::_read_binary_le(std::istream & in, std::streampos start_pos)
	{
		std::streampos pos = start_pos;
		for (auto elem = m_elements; elem && in; elem = elem->next)
		{
			elem->offset = pos;
			if (elem->name == "vertex")
			{
				_read_vertex(in, elem);
			}
			else if (elem->name == "face")
			{

			}
			else if (elem->name == "tristrip")
			{

			}
			// skip element data
			else if (elem->is_variant)
			{
				unsigned s = 0;
				IPlyReader **tail = &elem->readers;
				for (auto prop = elem->properties; prop; prop = prop->next)
				{
					if (prop->type != PLY_LIST)
						s += prop->size;
					else {
						IPlyReader *r1 = new CPlyIgnoreSize(s);
						*tail = r1;
						auto s1 = (prop->size & 0xFF0000) >> 16;
						auto s2 = (prop->size & 0xFF00) >> 8;
						IPlyReader *r2 = new CPlyIgnoreList(s1, s2);
						r1->next = r2;
						tail = &r2->next;
						s = 0;
					}
				}
				in.seekg(pos);
				for (auto r = elem->readers; r; r = r->next)
				{
					(*r)(in);
				}
				pos = in.tellg();
			}
			else {
				pos += elem->size;
			}
		}
		return true;
	}

	bool CPlyFile::_read_binary_be(std::istream & in)
	{
		m_error = PLY_NOT_SUPPORT_BID_ENDIAN;
		return false;
	}

	bool CPlyFile::_read_ascii(std::istream & in)
	{
		m_error = PLY_NOT_SUPPORT_ASCII;
		return false;
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