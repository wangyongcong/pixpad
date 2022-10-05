#include "ply.h"
#include <sstream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <cassert>
#include "common/utility.h"
#include "common/memory.h"
#include "stb/stb_log.h"

namespace wyc
{
	class PlyProperty
	{
	public:
		PlyProperty()
		: next(nullptr)
		, type(PLY_NULL)
		, size(0)
		{
		}
		PlyProperty *next;
		std::string name;
		PLY_PROPERTY_TYPE type;
		unsigned size;
	};

	class PlyElement
	{
	public:
		PlyElement()
			: next(nullptr)
			, count(0)
			, size(0)
			, properties(nullptr)
			, is_variant(false)
			, chunk_size(0)
		{
		}
		~PlyElement()
		{
			while (properties) {
				auto to_del = properties;
				properties = properties->next;
				wyc_delete(to_del);
			}
		}
		PlyElement *next;
		std::string name;
		unsigned count;
		unsigned size;
		PlyProperty *properties;
		bool is_variant;
		std::streamoff chunk_size;
	};

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
		virtual ~IPlyReader() = default;
		virtual bool operator() (std::istream &in) = 0;
		virtual void read_more(unsigned sz) {};
		IPlyReader *next;
	};

	static void clear_readers(IPlyReader *readers) 
	{
		auto iter = readers;
		while (iter) {
			auto _next = iter->next;
			wyc_delete(iter);
			iter = _next;
		}
	};

	class CPlyIgnoreSize : public IPlyReader
	{
	public:
		CPlyIgnoreSize(std::streampos sz)
			: m_size(sz)
		{
		}
		bool operator() (std::istream &in) override {
			in.ignore(m_size);
			return bool(in);
		}
		void read_more(unsigned sz) override {
			m_size += sz;
		}
	private:
		std::streampos m_size;
	};

	class CPlyReadFloat : public IPlyReader
	{
	public:
		CPlyReadFloat(unsigned count, float *out_buf, unsigned offset, unsigned stride)
			: m_stride(stride)
		{
			m_out_buf = (char*)(out_buf + offset);
			m_size = sizeof(float) * count;
		}
		bool operator() (std::istream &in) override {
			in.read(m_out_buf, m_size);
			m_out_buf += m_stride;
			return bool(in);
		}
		void read_more(unsigned count) override {
			m_size += sizeof(float) * count;
		}
	private:
		char *m_out_buf;
		unsigned m_stride;
		unsigned m_size;
	};

	class CPlyReadInteger : public IPlyReader
	{
	public:
		CPlyReadInteger(uint8_t count, uint8_t elem_size, float *out_buf, unsigned offset, unsigned stride)
			: m_stride(stride)
			, m_count(count)
			, m_elem_size(elem_size)
		{
			assert(elem_size <= sizeof(unsigned));
			m_out_buf = (char*)(out_buf + offset);
			m_max_value = float((1ul << (elem_size * 8)) - 1);
		}
		bool operator() (std::istream &in) override
		{
			unsigned v = 0;
			float *out = (float*)m_out_buf;
			for (uint8_t i = 0; i < m_count; ++i)
			{
				in.read((char*)&v, m_elem_size);
				*out++ = v / m_max_value;
			}
			m_out_buf += m_stride;
			return bool(in);
		}
		void read_more(unsigned count) override
		{
			m_count += count;
		}
	private:
		char *m_out_buf;
		float m_max_value;
		uint8_t m_stride;
		uint8_t m_count;
		uint8_t m_elem_size;
	};

	class CPlyIgnoreList : public IPlyReader
	{
	public:
		CPlyIgnoreList(unsigned prop_size)
			: m_bytes_for_length((prop_size >> 24) & 0xFF)
			, m_element_size((prop_size >> 8) & 0xFF)
		{
		}

		bool operator() (std::istream &in) override
		{
			unsigned length = 0;
			assert(m_bytes_for_length <= sizeof(length));
			in.read((char*)&length, m_bytes_for_length);
			in.ignore(length * m_element_size);
			return bool(in);
		}

	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
	};

	class CPlyReadFace : public IPlyReader
	{
	public:
		CPlyReadFace(unsigned bytes_for_length, unsigned element_size, char *out_buf, size_t buf_size, unsigned stride)
			: m_bytes_for_length(bytes_for_length)
			, m_element_size(element_size)
			, m_out_buf(out_buf)
			, m_buf_end(out_buf + buf_size)
			, m_buf_stride(stride)
		{
			assert(m_bytes_for_length <= sizeof(unsigned));
			if(m_element_size == stride)
			{
				m_reader = &CPlyReadFace::fit;
			}
			else if(m_element_size == sizeof(uint16_t) && m_buf_stride == sizeof(uint32_t))
			{
				m_reader = &CPlyReadFace::read_16_32;
			}
			else if(m_element_size == sizeof(uint32_t) && m_buf_stride == sizeof(uint16_t))
			{
				m_reader = &CPlyReadFace::read_32_16;
			}
			else
			{
				assert(0);
			}
		}

		bool operator() (std::istream &in) override
		{
			if(m_out_buf + m_buf_stride * 3 > m_buf_end)
				return false;
			unsigned length = 0;
			in.read((char*)&length, m_bytes_for_length);
			if (length != 3) {
				// if it's not a triangle, just ignore it
				// TODO: it's better to triangulate the face
				in.ignore(length * m_element_size);
			}
			else
			{
				(this->*m_reader)(in);
			}
			return bool(in);
		}

		void fit(std::istream &in)
		{
			unsigned sz = 3 * m_element_size;
			in.read(m_out_buf, sz);
			m_out_buf += sz;
		}

		void read_16_32(std::istream &in)
		{
			uint16_t value[3];
			in.read((char*)value, sizeof(uint16_t) * 3);
			uint32_t *buf = (uint32_t*)m_out_buf;
			buf[0] = value[0];
			buf[1] = value[1];
			buf[2] = value[2];
			m_out_buf += m_buf_stride * 3;
		}

		void read_32_16(std::istream &in)
		{
			uint32_t value[3];
			in.read((char*)value, sizeof(uint32_t) * 3);
			uint16_t *buf = (uint16_t*)m_out_buf;
			buf[0] = (uint16_t)value[0];
			buf[1] = (uint16_t)value[1];
			buf[2] = (uint16_t)value[2];
			m_out_buf += m_buf_stride * 3;
		}

	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
		char *m_out_buf;
		char *m_buf_end;
		size_t m_buf_stride;
		void (CPlyReadFace::*m_reader)(std::istream&);
	};

	class CPlyCountFace : public IPlyReader
	{
	public:
		CPlyCountFace(unsigned bytes_for_length, unsigned element_size, unsigned *count)
			: m_bytes_for_length(bytes_for_length)
			, m_element_size(element_size)
			, m_face_count(count)
		{
			assert(m_bytes_for_length <= sizeof(unsigned));
		}

		bool operator() (std::istream &in) override
		{
			unsigned length = 0;
			in.read((char*)&length, m_bytes_for_length);
			if (length == 3) {
				// ignore non-triangle face
				*m_face_count += 1;
			}
			in.ignore(length * m_element_size);
			return bool(in);
		}

	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
		unsigned *m_face_count;
	};

	class CPlyReadTristrip : public IPlyReader
	{
	public:
		CPlyReadTristrip(unsigned bytes_for_length, unsigned element_size, char *out_buf, size_t buf_size, unsigned stride)
			: m_bytes_for_length(bytes_for_length)
			, m_element_size(element_size)
			, m_out_buf(out_buf)
			, m_buf_end(out_buf + buf_size)
			, m_buf_stride(stride)
		{
			assert(m_bytes_for_length <= sizeof(unsigned));
			assert(m_element_size <= sizeof(unsigned));
			if(m_buf_stride == sizeof(uint16_t))
			{
				m_setter = &CPlyReadTristrip::set16;
			}
			else
			{
				assert(m_buf_stride == sizeof(uint32_t));
				m_setter = &CPlyReadTristrip::set32;
			}
		}

		bool operator() (std::istream &in) override
		{
			unsigned indices_count = 0;
			in.read((char*)&indices_count, m_bytes_for_length);
			int k = 0, p1 = -1, p2 = -1;
			bool flip = true;
			for (auto i = 0u; i < indices_count; ++i) {
				if (!in)
					return false;
				in.read((char*)&k, m_element_size);
				if (k < 0) {
					p1 = p2 = -1;
					flip = true;
					continue;
				}
				if (p1 == -1) {
					p1 = k;
					continue;
				}
				if (p2 == -1) {
					p2 = k;
					continue;
				}
				if(m_out_buf + m_buf_stride * 3 > m_buf_end)
					return false;
				flip = !flip;
				if (flip) {
					(this->*m_setter)(p2);
					(this->*m_setter)(p1);
				}
				else {
					(this->*m_setter)(p1);
					(this->*m_setter)(p2);
				}
				(this->*m_setter)(k);
				p1 = p2;
				p2 = k;
			}
			return bool(in);
		}

		void set16(int v)
		{
			*(uint16_t*)m_out_buf = (uint16_t)v;
			m_out_buf += m_buf_stride;
		}

		void set32(int v)
		{
			*(uint32_t*)m_out_buf = (uint32_t)v;
			m_out_buf += m_buf_stride;
		}

	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
		char *m_out_buf;
		char *m_buf_end;
		size_t m_buf_stride;
		void (CPlyReadTristrip::*m_setter)(int);
	};

	class CPlyCountTristrip : public IPlyReader
	{
	public:
		CPlyCountTristrip(unsigned bytes_for_length, unsigned element_size, unsigned *count)
			: m_bytes_for_length(bytes_for_length)
			, m_element_size(element_size)
			, m_face_count(count)
		{
			assert(m_bytes_for_length <= sizeof(unsigned));
			assert(m_element_size <= sizeof(int));
		}

		bool operator() (std::istream &in) override
		{
			unsigned length = 0;
			in.read((char*)&length, m_bytes_for_length);
			int k = 0, c = 0;
			for (unsigned i = 0u; i < length; ++i) {
				in.read((char*)&k, m_element_size);
				if (k < 0) {
					if(c > 2)
						*m_face_count += c - 2;
					c = 0;
				}
				else c += 1;
			}
			// *m_face_count *= 3;
			return bool(in);
		}

	private:
		unsigned m_bytes_for_length;
		unsigned m_element_size;
		unsigned *m_face_count;
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
		: m_data_pos(0)
		, m_error(PLY_NO_ERROR)
		, m_elements(nullptr)
		, m_face_count(0)
		, m_is_binary(false)
		, m_is_little_endian(false)
		, m_is_face_counted(false)
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
			wyc_delete(to_del);
		}
	}

	const char * CPlyFile::get_error_desc() const
	{
		static const char *s_ply_error_desc[] = {
			"no error",
			"file not found",
			"invalid ply file",
			"unknown format",
			"invalid property",
			"not support ascii format",
			"not support big endian format",
		};
		return s_ply_error_desc[m_error];
	}

	CPlyFile::PropertyIterator::PropertyIterator(const PlyProperty* prop)
		: m_property(prop)
	{
	}

	CPlyFile::PropertyIterator& CPlyFile::PropertyIterator::operator++()
	{
		if(m_property) m_property = m_property->next;
		return *this;
	}

	CPlyFile::PropertyIterator CPlyFile::PropertyIterator::operator++(int)
	{
		if(m_property)
		{
			PropertyIterator ret(m_property);
			m_property = m_property->next;
			return ret;
		}
		return {nullptr};
	}

	CPlyFile::PropertyIterator& CPlyFile::PropertyIterator::operator+=(int c)
	{
		for(int i=0; i<c; ++i)
		{
			if(!m_property)
			{
				break;
			}
			m_property = m_property->next;
		}
		return *this;
	}

	bool CPlyFile::PropertyIterator::is_vector(const char* x, const char* y, const char* z) const
	{
		auto prop = m_property;
		auto prop_type = prop->type;
		if(!prop || prop->name != x)
			return false;
		prop = prop->next;
		if(!prop || prop->name != y || prop->type != prop_type)
			return false;
		prop = prop->next;
		if(!prop || prop->name != z || prop->type != prop_type)
			return false;
		return true;
	}

	bool CPlyFile::PropertyIterator::is_float() const
	{
		return m_property->type == PLY_FLOAT;
	}

	bool CPlyFile::PropertyIterator::is_integer() const
	{
		return m_property->type == PLY_INTEGER;
	}

	unsigned CPlyFile::PropertyIterator::size() const
	{
		return m_property ? m_property->size : 0;
	}

	const std::string& CPlyFile::PropertyIterator::name() const
	{
		return m_property ? m_property->name : g_empty_string;
	}

	CPlyFile::PropertyIterator CPlyFile::get_vertex_property() const
	{
		auto element = _find_element("vertex");
		return {element ? element->properties : nullptr};
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

	unsigned CPlyFile::vertex_count() const
	{
		auto *elem = _find_element("vertex");
		return elem ? elem->count : 0;
	}

	unsigned CPlyFile::face_count()
	{
		if(!m_is_face_counted)
		{
			m_face_count = 0;
			_read_face(nullptr, 0, 0, m_face_count);
			m_is_face_counted = true;
		}
		return m_face_count;
	}

	const PlyElement* CPlyFile::_find_element(const char *elem_name) const {
		auto elem = m_elements;
		while (elem) {
			if (elem->name == elem_name)
				return elem;
			elem = elem->next;
		}
		return nullptr;
	}

	bool CPlyFile::_read_vector3(float * vector3, unsigned & count, unsigned stride, const char * v1, const char * v2, const char * v3)
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
		PlyProperty *prop;
		for (prop = elem->properties; prop && prop->name != v1; prop = prop->next)
		{
			pos += prop->size;
		}
		if (!prop)
			return false;
		auto prop_type = prop->type;
		prop = prop->next;
		if (!prop || prop->name != v2 || prop->type != prop_type)
			return false;
		prop = prop->next;
		if (!prop || prop->name != v3 || prop->type != prop_type)
			return false;
		unsigned sz = prop->size * 3, c = 0;
		tail = elem->size - sz;
		auto out = vector3;
		if (prop_type == PLY_FLOAT) {
			if (prop->size != sizeof(float))
				return false;
			m_stream.ignore(pos);
			for (c = 0; c < elem->count; ++c && c < count, out += stride)
			{
				m_stream.read((char*)out, sz);
				m_stream.ignore(tail);
			}
			count = c;
		}
		else if (prop_type == PLY_INTEGER) {
			unsigned max;
			switch (prop->size) {
			case 1:
				max = std::numeric_limits<char>::max();
				break;
			case 2:
				max = std::numeric_limits<short>::max();
				break;
			case 4:
				max = std::numeric_limits<int>::max();
				break;
			default:
				return false;
			}
			m_stream.ignore(pos);
			unsigned tmp = 0;
			for (c = 0; c < elem->count; ++c && c < count)
			{
				m_stream.read((char*)&tmp, prop->size);
				*out++ = float(tmp) / max;
				m_stream.read((char*)&tmp, prop->size);
				*out++ = float(tmp) / max;
				m_stream.read((char*)&tmp, prop->size);
				*out++ = float(tmp) / max;
				m_stream.ignore(tail);
			}
		}
		else {
			return false;
		}
		count = c;
		return true;
	}

	bool CPlyFile::_find_vector3(PLY_PROPERTY_TYPE type, const char * v1, const char * v2, const char * v3) const
	{
		auto *elem = _find_element("vertex");
		if (!elem)
			return false;
		PlyProperty *prop;
		for (prop = elem->properties; prop && prop->name != v1; prop = prop->next);
		if (!prop || prop->type != type)
			return false;
		prop = prop->next;
		if (!prop || prop->name != v2 || prop->type != type)
			return false;
		prop = prop->next;
		if (!prop || prop->name != v3 || prop->type != type)
			return false;
		return true;
	}

	bool CPlyFile::read_vertex(float * vertex, unsigned & count, const std::string& layout, unsigned stride)
	{
		auto elem = _locate_element("vertex");
		if (!elem)
			return false;
		// std::string tok;
		PLY_PROPERTY_TYPE prev_type = PLY_NULL;
		IPlyReader *readers = nullptr, *r = nullptr;
		IPlyReader **tail = &readers;
		unsigned prev_offset = 0, prev_size = 0;
		StringSplitter splitter(layout, ',');
		for (auto prop = elem->properties; prop; prop = prop->next)
		{
			if (prop->type == PLY_LIST) {
				r = wyc_new(CPlyIgnoreList, prop->size);
				*tail = r;
				tail = &r->next;
				prev_type = PLY_LIST;
				continue;
			}
			bool is_output_prop = false;
			unsigned prop_offset = 0;
			for (auto iter : splitter)
			{
				if(prop->name == iter)
				{
					is_output_prop = true;
					break;
				}
				prop_offset += 1;
			}
			if (is_output_prop)
			{
				if (prop->type == PLY_FLOAT) {
					if (prev_type == prop->type && prev_size == prop->size && prop_offset == prev_offset + 1)
					{
						r->read_more(1);
						prev_offset += 1;
					}
					else {
						r = wyc_new(CPlyReadFloat, 1, vertex, prop_offset, stride);
						*tail = r;
						tail = &r->next;
						prev_type = prop->type;
						prev_size = prop->size;
						prev_offset = prop_offset;
					}
				}
				else if (prop->type == PLY_INTEGER) {
					if (prev_type == prop->type && prev_size == prop->size && prop_offset == prev_offset + 1)
					{
						r->read_more(1);
						prev_offset += 1;
					}
					else {
						r = wyc_new(CPlyReadInteger, 1, prop->size, vertex, prop_offset, stride);
						*tail = r;
						tail = &r->next;
						prev_type = prop->type;
						prev_size = prop->size;
						prev_offset = prop_offset;
					}
				}
				else {
					assert(0 && "should not arrive here");
				}
			}
			else {
				// property not found, ignore
				if (r && prev_type == PLY_NULL) {
					r->read_more(prop->size);
				}
				else {
					r = wyc_new(CPlyIgnoreSize, prop->size);
					*tail = r;
					tail = &r->next;
					prev_type = PLY_NULL;
				}
			}
		} // for property

		if (elem->count < count)
			count = elem->count;
		unsigned c = 0;
		for (; c < count && m_stream; ++c) {
			for (auto r = readers; r; r = r->next)
			{
				if (!(*r)(m_stream))
					break;
			}
		}
		count = c;
		clear_readers(readers);
		return true;
	}

	bool CPlyFile::read_vertex(char* buffer, size_t buffer_size)
	{
		auto elem = _locate_element("vertex");
		if (!elem)
			return false;
		size_t size = elem->count * elem->size;
		if(buffer_size < size)
			return false;
		m_stream.read(buffer, buffer_size);
		return !m_stream.fail();
	}

	bool CPlyFile::read_face(char* buffer, size_t buffer_size, unsigned stride)
	{
		unsigned total_count = 0;
		return _read_face(buffer, buffer_size, stride, total_count);
	}

	bool CPlyFile::_read_face(char *buffer, size_t buffer_size, unsigned stride, unsigned &count)
	{
		PlyElement *elem;
		bool is_tristrip = false, has_vertex_indices = false;
		for (elem = m_elements; elem; elem = elem->next)
		{
			if (elem->name == "face") {
				is_tristrip = false;
				if(!buffer)
				{
					count = elem->count;
					return true;
				}
				break;
			}
			if (elem->name == "tristrips") {
				is_tristrip = true;
				break;
			}
		}
		if (!elem) {
			// faces not found
			return false;
		}
		elem = _locate_element(elem->name.c_str());
		IPlyReader *readers = nullptr;
		IPlyReader **tail = &readers;
		unsigned ignore_size = 0;
		PlyProperty *prop = elem->properties;
		for (; prop; prop = prop->next)
		{
			if (prop->name == "vertex_indices") {
				IPlyReader *r;
				if (ignore_size) {
					r = wyc_new(CPlyIgnoreSize, ignore_size);
					tail = &r->next;
					ignore_size = 0;
				}
				unsigned sz1 = (prop->size >> 24) & 0xFF;
				unsigned sz2 = (prop->size >> 8) & 0xFF;
				if (buffer) {
					if (is_tristrip)
						r = wyc_new(CPlyReadTristrip, sz1, sz2, buffer, buffer_size, stride);
					else
						r = wyc_new(CPlyReadFace, sz1, sz2, buffer, buffer_size, stride);
				}
				else {
					if (is_tristrip)
						r = wyc_new(CPlyCountTristrip, sz1, sz2, &count);
					else
						r = wyc_new(CPlyCountFace, sz1, sz2, &count);
				}
				*tail = r;
				tail = &r->next;
				has_vertex_indices = true;
			}
			else if(prop->type == PLY_LIST) {
				if (ignore_size) {
					auto r = wyc_new(CPlyIgnoreSize, ignore_size);
					*tail = r;
					tail = &r->next;
					ignore_size = 0;
				}
				auto r = wyc_new(CPlyIgnoreList, prop->size);
				*tail = r;
				tail = &r->next;
			}
			else {
				ignore_size += prop->size;
			}
		}
		// last ignore
		if (ignore_size)
		{
			auto r = wyc_new(CPlyIgnoreSize, ignore_size);
			*tail = r;
			tail = &r->next;
		}
		if (!has_vertex_indices) {
			clear_readers(readers);
			return false;
		}
		if (!readers->next) {
			// only one reader
			for (auto i = 0u; i < elem->count; ++i) {
				if (!(*readers)(m_stream))
					break;
			}
		}
		else {
			for (auto i = 0u; i < elem->count && m_stream; ++i) {
				for (auto r = readers; r; r = r->next) {
					if (!(*r)(m_stream))
						break;
				}
			}
		}
		clear_readers(readers);
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
					sz = (int)std::stol(sub);
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
					sz = (int)std::stol(sub);
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
					sz = (int)std::stol(sub);
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
//		constexpr auto max_size = std::numeric_limits<std::streamsize>::max();
		if (m_elements)
		{
			_clear();
		}
		unsigned count = 0;
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
				if (value == "ascii") {
					m_is_binary = false;
					m_error = PLY_NOT_SUPPORT_ASCII;
				}
				else if (value == "binary_big_endian")
				{
					m_is_binary = true;
					m_is_little_endian = false;
					m_error = PLY_NOT_SUPPORT_BID_ENDIAN;
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
					cur_elem = wyc_new(PlyElement);
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
				PlyProperty *prop = wyc_new(PlyProperty);
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
				if(s > 0) {
					IPlyReader *r1 = wyc_new(CPlyIgnoreSize, s);
					*tail = r1;
					tail = &r1->next;
				}
				IPlyReader *r2 = wyc_new(CPlyIgnoreList, prop->size);
				*tail = r2;
				tail = &r2->next;
				s = 0;
			}
		}
		m_stream.seekg(m_data_pos + pos);
		for (unsigned i = 0; i < elem->count; ++i) {
			for (auto r = readers; r; r = r->next)
			{
				(*r)(m_stream);
				if (!m_stream) {
					clear_readers(readers);
					return 0;
				}
			}
		}
		auto end_pos = m_stream.tellg();
		clear_readers(readers);
		return end_pos - pos;
	}

} // namespace wyc
