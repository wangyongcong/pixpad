#include "ply.h"
#include <sstream>
#include <limits>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cassert>
#include "common/utility.h"
#include "common/memory.h"
#include "stb/stb_log.h"

namespace wyc
{
	struct PlyListData
	{
		PlyListData* next;
		char* data;
		size_t capacity;
		size_t size;

		PlyListData() : next(nullptr), data(nullptr), capacity(0), size(0)
		{
		}

		~PlyListData()
		{
			if(data) wyc_free(data);
		}

		void ensure_capacity(unsigned length, unsigned item_size)
		{
			size_t new_size = size + length;
			if(new_size > capacity)
			{
				size_t increase = std::max(new_size - capacity, capacity / 2);
				capacity += increase;
				data = (char*)wyc_realloc(data, capacity * item_size);
			}
		}
	};

	class PlyPropertyImpl : public PlyProperty
	{
	public:
		PlyPropertyImpl() : PlyProperty(), list_data(nullptr)
		{
		}
		PlyListData* list_data;
	};

	class PlyElementImpl : public PlyElement
	{
	public:
		PlyElementImpl()
			: PlyElement()
			, data(nullptr)
			, data_size(0)
			, list_data(nullptr)
			, list_count(0)
		{
		}
		~PlyElementImpl()
		{
			while (properties) {
				auto to_del = (PlyPropertyImpl*)properties;
				properties = properties->next;
				wyc_delete(to_del);
			}
			if(data)
			{
				wyc_free(data);
			}
			while(list_data)
			{
				auto to_del = list_data;
				list_data = list_data->next;
				wyc_delete(to_del);
			}
		}
		char* data;
		unsigned data_size;
		PlyListData* list_data;
		unsigned list_count;
	};

#define GET_PROPERTY(e) (PlyPropertyImpl*)((e)->properties)
#define NEXT_PROPERTY(p) (PlyPropertyImpl*)((p)->next)
#define NEXT_ELEMENT(e) (PlyElementImpl*)((e)->next)
#define FIND_ELEMENT(t) (const PlyElementImpl*)find_element(PLY_TAGS[(t)])

	bool PlyProperty::is_vector(const char* x, const char* y, const char* z, PlyProperty const** last) const
	{
		auto prop = this;
		auto prop_type = prop->type;
		if(!prop || prop->name != x)
			return false;
		prop = prop->next;
		if(!prop || prop->name != y || prop->type != prop_type)
			return false;
		prop = prop->next;
		if(!prop || prop->name != z || prop->type != prop_type)
			return false;
		if(last)
			*last = prop->next;
		return true;
	}

	const char* PLY_TAGS[] = {
		"comment",
		"format",
		"element",
		"property",
		"end_header",
		"vertex",
		"face",
		"tristrips",
		"vertex_indices",
	};

	enum PLY_TAG_TYPE {
		// file section name
		COMMENT = 0,
		FORMAT,
		ELEMENT,
		PROPERTY,
		END_HEADER,
		// element or property name
		PLY_VERTEX,
		PLY_FACE,
		PLY_TRISTRIPS,
		PLY_VERTEX_INDICES,
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
		: m_path(file_path)
		, m_data_pos(0)
		, m_error(PLY_NO_ERROR)
		, m_elements(nullptr)
		, m_cached_face_count(std::numeric_limits<decltype(m_cached_face_count)>::max())
		, m_is_binary(false)
		, m_is_little_endian(false)
		, m_is_data_loaded(false)
	{
		preload(file_path);
	}

	CPlyFile::~CPlyFile()
	{
		m_stream.close();
		clear();
	}

	void CPlyFile::clear()
	{
		while (m_elements) 
		{
			auto to_del = (PlyElementImpl*)m_elements;
			m_elements = m_elements->next;
			wyc_delete(to_del);
		}
	}

	const char * CPlyFile::get_error_desc() const
	{
		static const char *s_ply_error_desc[] = {
			"No error",
			"File not found",
			"Invalid ply file",
			"Unknown format",
			"Invalid property",
			"Not support ascii format",
			"Not support big endian format",
			"Element contain multiple list properties",
			"Size of list length is more then 4 bytes",
			"Size of list item length is invalid",
			"I/O interrupt unexpectedly",
			"Invalid face list",
		};
		return s_ply_error_desc[m_error];
	}

	void CPlyFile::detail(std::ostream & out) const
	{
		out << "[PLY] " << m_path << std::endl;
		if (!m_elements)
		{
			out << "No elements" << std::endl;
			return;
		}
		for (auto elem = m_elements; elem; elem = elem->next)
		{
			out << elem->name << "(" << elem->count << "/" << elem->size << ")" << std::endl;
			for (auto prop = elem->properties; prop; prop = prop->next)
			{
				out << "    " << prop->name << "(";
				if (prop->type == PLY_LIST)
				{
					unsigned length_size = prop->length_size;
					unsigned item_size = prop->item_size;
					out << "L" << length_size << "/" << item_size;
				}
				else if(prop->type == PLY_INTEGER)
					out << "i" << prop->size;
				else
					out << "f" << prop->size;
				out << ")" << std::endl;
			}
		}
	}

	void CPlyFile::detail() const
	{
		std::ostringstream ss;
		detail(ss);
		log_info(ss.str());
	}

	unsigned CPlyFile::vertex_count() const
	{
		auto elem = FIND_ELEMENT(PLY_VERTEX);
		return elem ? elem->count : 0;
	}

	const PlyElement* CPlyFile::find_element(const char *elem_name) const {
		auto elem = m_elements;
		while (elem) {
			if (elem->name == elem_name)
				return elem;
			elem = elem->next;
		}
		return nullptr;
	}

	const PlyProperty* CPlyFile::get_vertex_property() const
	{
		auto elem = FIND_ELEMENT(PLY_VERTEX);
		return elem ? elem->properties : nullptr;
	}

	unsigned CPlyFile::read_vertex(char* buffer, size_t buffer_size) const
	{
		auto elem = FIND_ELEMENT(PLY_VERTEX);
		if(!elem || !elem->data)
		{
			return false;
		}
		unsigned count = (unsigned)(buffer_size / elem->size);
		count = std::min<unsigned>(count, elem->count);
		unsigned size = count * elem->size;
		assert(size <= elem->data_size);
		memcpy(buffer, elem->data, size);
		return count;
	}

	template<class T>
	unsigned count_tristrip(const T* data, unsigned size)
	{
		unsigned total = 0;
		unsigned start = 0;
		for(unsigned i = 0; i < size; ++i)
		{
			if(data[i] < 0)
			{
				auto c = i - start;
				if(c > 2)
					total += c - 2;
				start = i + 1;
			}
		}
		return total;
	}

	unsigned CPlyFile::face_count() const
	{
		if(!m_is_data_loaded)
		{
			return 0;
		}
		if(m_cached_face_count < std::numeric_limits<decltype(m_cached_face_count)>::max())
		{
			return m_cached_face_count;
		}
		m_cached_face_count = 0;
		bool is_tristrips = false;
		auto *elem = FIND_ELEMENT(PLY_FACE);
		if (!elem)
		{
			elem = FIND_ELEMENT(PLY_TRISTRIPS);
			if(!elem)
			{
				return 0;
			}
			is_tristrips = true;
		}

		auto prop = GET_PROPERTY(elem);
		unsigned offset = 0;
		while(prop && prop->name != PLY_TAGS[PLY_VERTEX_INDICES])
		{
			offset += prop->size;
			prop = NEXT_PROPERTY(prop);
		}
		if(!prop || prop->type != PLY_LIST || prop->item_type != PLY_INTEGER)
		{
			log_error("[PLY] Invalid face/tristrip list");
			return 0;
		}

		char* list_length = elem->data + offset;
		auto* list_data = prop->list_data;
		assert(list_data);
		unsigned item_size = prop->item_size;
		unsigned total = 0;
		if(!is_tristrips)
		{
			for(unsigned i = 0; i < elem->count; ++i)
			{
				total += *(unsigned*)list_length - 2;
				list_length += elem->size;
			}
		}
		else if(item_size == sizeof(int16_t))
		{
			int16_t* data = (int16_t*)list_data->data;
			for(unsigned i = 0; i < elem->count; ++i)
			{
				auto length = *(unsigned*)list_length;
				list_length += elem->size;
				total += count_tristrip<int16_t>(data, length);
				data += length;
			}
			assert(data <= (int16_t*)list_data->data + list_data->size);
		}
		else if(item_size == sizeof(int32_t))
		{
			int32_t* data = (int32_t*)list_data->data;
			for(unsigned i = 0; i < elem->count; ++i)
			{
				auto length = *(unsigned*)list_length;
				list_length += elem->size;
				total += count_tristrip<int32_t>(data, length);
				data += length;
			}
			assert(data <= (int32_t*)list_data->data + list_data->size);
		}
		else
		{
			log_error("[PLY] tristrip index is neither int16 nor int32");
		}
		m_cached_face_count = total;
		return total;
	}

	template<class T1, class T2>
	unsigned copy_face(T1* dst, size_t dst_size, T2* src, size_t src_size, char* length_list, unsigned length_stride, unsigned length_count)
	{
		unsigned index_count = 0;
		T2* src_end = src + src_size;
		for(unsigned i = 0; i < length_count; ++i)
		{
			unsigned length = *(unsigned*)length_list;
			length_list += length_stride;
			if(src + length > src_end)
				break;
			unsigned count = (length > 2 ? length - 2 : 0) * 3;
			if(index_count + count > dst_size)
				break;
			for(unsigned j = 2; j < length; ++j)
			{
				*dst++ = (T1)src[0];
				*dst++ = (T1)src[j-1];
				*dst++ = (T1)src[j];
			}
			src += length;
			index_count += count;
		}
		return index_count;
	}

	// tristrip element format
	// -1 means end of triangle strips
	// e.g index list: 0 1 2 3 -1 4 5 6 7
	// indicate 4 triangles:
	// (0, 1, 2)
	// (1, 2, 3)
	// (4, 5, 6)
	// (5, 6, 7)
	template<class T1, class T2>
	unsigned copy_tristrips(T1* dst, size_t dst_size, T2* src, size_t src_size, char* length_list, unsigned length_stride, unsigned length_count)
	{
		unsigned index_count = 0;
		T2* src_end = src + src_size;
		for(unsigned i = 0; i < length_count; ++i)
		{
			unsigned length = *(unsigned*)length_list;
			length_list += length_stride;
			if(src + length > src_end)
				break;
			if(index_count + 3 > dst_size)
				break;
			int p0 = 0, p1 = -1, p2 = -1;
			bool flip = true;
			for (unsigned j = 0; j < length; ++j)
			{
				p0 = (int)src[j];
				if (p0 < 0)
				{
					p1 = p2 = -1;
					flip = true;
					continue;
				}
				if (p1 == -1)
				{
					p1 = p0;
					continue;
				}
				if (p2 == -1)
				{
					p2 = p0;
					continue;
				}
				if (index_count + 3 > dst_size)
					break;
				flip = !flip;
				if (flip)
				{
					*dst++ = (T1)p2;
					*dst++ = (T1)p1;
				}
				else
				{
					*dst++ = (T1)p1;
					*dst++ = (T1)p2;
				}
				*dst++ = (T1)p0;
				index_count += 3;
				p1 = p2;
				p2 = p0;
			}
			src += length;
		}
		return index_count;
	}

	unsigned CPlyFile::read_face(char* buffer, size_t buffer_size, unsigned index_size) const
	{
		bool is_short_index = index_size == sizeof(uint16_t);
		if(!is_short_index && index_size != sizeof(uint32_t))
		{
			log_warning("[PLY] Index buffer should be uint16 or uint32");
			return 0;
		}
		bool is_tristrips = false;
		auto *elem = FIND_ELEMENT(PLY_FACE);
		if (!elem)
		{
			elem = FIND_ELEMENT(PLY_TRISTRIPS);
			if(!elem)
			{
				log_warning("[PLY] no face data");
				return 0;
			}
			is_tristrips = true;
		}

		auto prop = GET_PROPERTY(elem);
		unsigned offset = 0;
		while(prop && prop->name != PLY_TAGS[PLY_VERTEX_INDICES])
		{
			offset += prop->size;
			prop = NEXT_PROPERTY(prop);
		}
		if(!prop || prop->type != PLY_LIST || prop->item_type != PLY_INTEGER)
		{
			log_error("[PLY] Invalid face/tristrip list");
			return 0;
		}

		char* list_length = elem->data + offset;
		auto* list_data = prop->list_data;
		assert(list_data);
		unsigned item_size = prop->item_size;
		bool is_short_item = item_size == sizeof(uint16_t);
		if(!is_short_item && item_size != sizeof(uint32_t))
		{
			log_error("[PLY] Face and tristrip index is neither int16 nor int32");
			return 0;
		}
		size_t size = buffer_size / index_size;
		unsigned read_index = 0;
		if(!is_tristrips)
		{
			if (is_short_index)
			{
				if (!is_short_item)
					read_index = copy_face((uint16_t*)buffer, size, (uint32_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
				else
					read_index = copy_face((uint16_t*)buffer, size, (uint16_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
			}
			else
			{
				if (!is_short_item)
					read_index = copy_face((uint32_t*)buffer, size, (uint32_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
				else
					read_index = copy_face((uint16_t*)buffer, size, (uint16_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
			}
		}
		else if(is_short_index)
		{
			if(!is_short_item)
				read_index = copy_tristrips((uint16_t*)buffer, size, (int32_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
			else
				read_index = copy_tristrips((uint16_t*)buffer, size, (int16_t*)list_data->data, list_data->size, list_length, elem->size, elem->count);
		}
		else
		{
			if(!is_short_item)
				read_index = copy_tristrips((uint32_t*)buffer, size, (int32_t*)elem->data, elem->size, list_length, elem->size, elem->count);
			else
				read_index = copy_tristrips((uint32_t*)buffer, size, (int16_t*)elem->data, elem->size, list_length, elem->size, elem->count);
		}
		return read_index;
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
	static std::pair<EPlyPropertyType, uint8_t> ply_property_type(const std::string &type)
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

	bool CPlyFile::preload(const std::string & path)
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
		if (m_elements)
		{
			clear();
		}
		unsigned count = 0;
		PlyElementImpl *cur_elem = nullptr;
		PlyElementImpl **tail = (PlyElementImpl**)&m_elements;
		PlyPropertyImpl **prop_tail = nullptr;
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
					cur_elem = wyc_new(PlyElementImpl);
					cur_elem->count = count;
					cur_elem->name = value;
					*tail = cur_elem;
					tail = (PlyElementImpl**)&cur_elem->next;
					prop_tail = (PlyPropertyImpl**)&cur_elem->properties;
				}
			}
			else if (value == PLY_TAGS[PROPERTY])
			{
				// property type
				line >> value;
				if (line.fail() || value.empty())
					continue;
				PlyPropertyImpl *prop = wyc_new(PlyPropertyImpl);
				*prop_tail = prop;
				prop_tail = (PlyPropertyImpl**)&prop->next;
				cur_elem->property_count += 1;
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
					// prop->size = (t1.second << 24) | (t1.first << 16) | (t2.second << 8) | t2.first;
					prop->length_size = t1.second;
					prop->item_size = t2.second;
					prop->item_type = t2.first;
					cur_elem->size += sizeof(unsigned);
					cur_elem->list_count += 1;
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


	bool CPlyFile::load()
	{
		if(m_is_data_loaded)
		{
			return true;
		}
		if(!m_stream.is_open())
		{
			m_error = PLY_INVALID_FILE;
			return false;
		}
		if (!m_stream)
		{
			m_stream.clear();
		}
		m_stream.seekg(m_data_pos);
		for(PlyElementImpl* elem = (PlyElementImpl*)m_elements; elem; elem = NEXT_ELEMENT(elem))
		{
			if(!m_stream)
			{
				m_error = PLY_IO_INTERRUPT;
				return false;
			}
			if(!elem->data)
			{
				elem->data_size = elem->size * elem->count;
				elem->data = (char*)wyc_malloc(elem->data_size);
			}
			if(!elem->list_count)
			{
				m_stream.read(elem->data, elem->data_size);
				continue;
			}

			typedef std::function<char*(std::istream&, char*)> Reader;
			std::vector<Reader> readers;
			readers.reserve(elem->property_count);
			unsigned fixed_size = 0;
			PlyListData** list_data_tail = &elem->list_data;
			for(PlyPropertyImpl* prop = GET_PROPERTY(elem); prop; prop = NEXT_PROPERTY(prop))
			{
				if (prop->type != PLY_LIST)
				{
					fixed_size += prop->size;
					continue;
				}
				if(fixed_size)
				{
					readers.emplace_back([fixed_size](std::istream& in, char* buf)
					{
						in.read(buf, fixed_size);
						return buf + fixed_size;
					});
				}
				// list data format
				// 3 0 1 2
				// ^ ^
				// | |____ the first index
				// |____ number of index
				// list may contain 3 or 4 indices
				// 3 indices indicate a triangle
				// 4 indices indicate a triangle fan
				// e.g 4 0 1 2 3
				// indicate 2 triangles: (0, 1, 2) and (0, 2, 3)
				const unsigned length_size = prop->length_size;
				const unsigned item_size = prop->item_size;
				if(length_size > sizeof(unsigned))
				{
					m_error = PLY_LIST_LENGTH_OVERFLOW;
					return false;
				}
				if(item_size == 0)
				{
					m_error = PLY_LIST_INVALID_ITEM_SIZE;
					return false;
				}
				PlyListData* list_data = wyc_new(PlyListData);
				prop->list_data = list_data;
				*list_data_tail = list_data;
				list_data_tail = &list_data->next;
				readers.emplace_back([length_size, item_size, list_data](std::istream& in, char* buf)
				{
					unsigned length = 0;
					in.read((char*)&length, length_size);
					*(unsigned*)buf = length;
					buf += sizeof(unsigned);
					list_data->ensure_capacity(length, item_size);
					in.read(list_data->data + list_data->size * item_size, length * item_size);
					list_data->size += length;
					return buf;
				});
			}
			if(fixed_size)
			{
				readers.emplace_back([fixed_size](std::istream& in, char* buf)
				{
					in.read(buf, fixed_size);
					return buf + fixed_size;
				});
			}
			char* data = elem->data;
			for(unsigned i=0; i<elem->count; ++i)
			{
				for (auto reader : readers)
				{
					if(!m_stream)
					{
						m_error = PLY_IO_INTERRUPT;
						return false;
					}
					data = reader(m_stream, data);
				}
			}
		}
		m_is_data_loaded = true;
		return true;
	}

} // namespace wyc
