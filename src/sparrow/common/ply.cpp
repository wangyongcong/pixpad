#include "ply.h"
#include <sstream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <cassert>
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


	class IPlyReader
	{
	public:
		IPlyReader()
			: next(nullptr)
		{
		}
		virtual void operator() (std::istream &in) = 0;
		IPlyReader *next;
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
		, m_is_binary(false)
		, m_is_little_endian(false)
		, m_data_pos(0)
	{
		_load(file_path);
	}

	CPlyFile::~CPlyFile()
	{
		m_stream.close();
		_clear();
	}

	void CPlyFile::_clear()
	{
		while (m_elements) {
			auto to_del = m_elements;
			m_elements = m_elements->next;
			delete to_del;
		}
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
					out << "L" << ((prop->size >> 24) & 0xFF) << "/" << ((prop->size >> 8) & 0xFF);
				else if(prop->type == PLY_INTEGER)
					out << "i" << prop->size;
				else
					out << "f" << prop->size;
				out << ")" << std::endl;
			}
		}
	}

	const PlyElement * CPlyFile::find_element(const std::string & name)
	{
		for (auto elem = m_elements; elem; elem = elem->next)
		{
			if (elem->name == name)
				return elem;
		}
		return nullptr;
	}

	bool CPlyFile::read_vertex_position(float * vector3, unsigned & count, unsigned stride)
	{
		auto elem = _locate_element("vertex");
		if (!elem || elem->is_variant) {
			count = 0;
			return false;
		}
		if (!vector3) {
			count = elem->count;
			return true;
		}
		std::streampos pos = 0, tail = 0;
		PlyProperty *prop_x;
		for (prop_x = elem->properties; prop_x && prop_x->name != "x"; prop_x = prop_x->next)
		{
			pos += prop_x->size;
		}
		tail = elem->size - pos;
		if (!prop_x || prop_x->type != PLY_FLOAT)
			return false;
		auto prop_y = prop_x->next;
		if (!prop_y || prop_y->name != "y" || prop_y->type != PLY_FLOAT)
			return false;
		auto prop_z = prop_y->next;
		if (!prop_z || prop_z->name != "z" || prop_z->type != PLY_FLOAT)
			return false;
		if (!m_is_binary) {
			m_error = PLY_NOT_SUPPORT_ASCII;
			return false;
		}
		if (!m_is_little_endian) {
			m_error = PLY_NOT_SUPPORT_BID_ENDIAN;
			return false;
		}
		static_assert(sizeof(float) == 4, "assume that float size is 4 bytes");
		constexpr unsigned sz = sizeof(float) * 3;		
		auto out = vector3;
		m_stream.ignore(pos);
		unsigned c;
		for (c = 0; c < elem->count; ++c && c < count, out += stride)
		{
			m_stream.read((char*)out, sz);
			m_stream.ignore(elem->size);
		}
		count = c;
		return true;
	}

	bool CPlyFile::read_face(unsigned * vertex_indices, unsigned & count)
	{
		auto elem = _locate_element("face");
		if (!elem || !elem->is_variant)
			return false;
		PlyProperty *prop = elem->properties;
		if (prop->name != "vertex_indices" || prop->type != PLY_LIST)
			return false;
		unsigned sz1 = (prop->size >> 24) & 0xFF;
		unsigned sz2 = (prop->size >> 8) & 0xFF;
		unsigned len = 0;
		if (!vertex_indices) {
			count = 0;
			for (unsigned i = 0; i < elem->count; ++i)
			{
				m_stream.read((char*)&len, sz1);
				if (len != 3)
					continue;
				count += 3;
				m_stream.ignore(len * sz2);
			}
			return true;
		}
		unsigned *out = vertex_indices;
		unsigned cnt = 0;
		for (unsigned i = 0; i < elem->count; ++i)
		{
			m_stream.read((char*)&len, sz1);
			if (len != 3)
				continue;
			cnt += 3;
			if (cnt > count) {
				cnt -= 3;
				break;
			}
			for (unsigned j = 0; j < len; ++j, ++out) {
				m_stream.read((char*)out, sz2);
			}
		}
		count = cnt;
		return true;
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
		m_stream.open(path, std::ios_base::binary);
		if (!m_stream.is_open())
		{
			m_error = PLY_FILE_NOT_FOUND;
			return false;
		}
		std::stringstream line;
		std::string value, type;
		std::getline(m_stream, value);
		if (value != "ply") {
			m_error = PLY_INVALID_FILE;
			return false;
		}
		unsigned count;
		constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
		if (m_elements)
			_clear();
		PlyElement *cur_elem = nullptr;
		PlyElement **tail = &m_elements;
		PlyProperty **prop_tail = nullptr;
		while (m_stream) {
			std::getline(m_stream, value);
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
					m_is_binary = false;
				else if (value == "binary_big_endian")
				{
					m_is_binary = true;
					m_is_little_endian = false;
				}
				else if (value == "binary_little_endian") {
					m_is_binary = true;
					m_is_little_endian = true;
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
					cur_elem->count = count;
					cur_elem->name = value;
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
					prop->size = (t1.second << 24) | (t1.first << 16) | (t2.second << 8) | t2.first;
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
		m_data_pos = m_stream.tellg();
		return true;
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

	PlyElement* CPlyFile::_locate_element(const char *elem_name)
	{
		assert(m_stream.is_open());
		if (!m_stream) {
			m_stream.clear();
		}
		std::streampos pos = 0;
		PlyElement *elem;
		for (elem = m_elements; elem && elem->name != elem_name; elem = elem->next)
		{
			if (!elem->chunk_size) {
				if (!elem->is_variant) 
					elem->chunk_size = elem->size * elem->count;
				else {
					elem->chunk_size = _calculate_chunk_size(elem, pos);
					if (!elem->chunk_size)
						return nullptr;
				}
			}
			pos += elem->chunk_size;
		}
		if (!elem) 
			return nullptr;
		m_stream.seekg(m_data_pos + pos);
		return elem;
	}

	std::streamoff CPlyFile::_calculate_chunk_size(PlyElement * elem, std::streampos pos)
	{
		unsigned s = 0;
		IPlyReader *readers = nullptr;
		IPlyReader **tail = &readers;
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
		auto clear_readers = [readers]() {
			auto iter = readers;
			while (iter) {
				auto _next = iter->next;
				delete iter;
				iter = _next;
			}
		};
		m_stream.seekg(pos);
		for (unsigned i = 0; i < elem->count; ++i) {
			for (auto r = readers; r; r = r->next)
			{
				(*r)(m_stream);
				if (!m_stream) {
					clear_readers();
					return 0;
				}
			}
		}
		auto end_pos = m_stream.tellg();
		clear_readers();
		return end_pos - pos;
	}

} // namespace wyc
