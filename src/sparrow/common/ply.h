#pragma once
#include <string>
#include <fstream>

namespace wyc
{
	enum PLY_ERROR 
	{
		PLY_NO_ERROR = 0,
		PLY_FILE_NOT_FOUND,
		PLY_INVALID_FILE,
		PLY_UNKNOWN_FORMAT,
		PLY_INVALID_PROPERTY,
		PLY_NOT_SUPPORT_ASCII,
		PLY_NOT_SUPPORT_BID_ENDIAN,
	};

	enum PLY_PROPERTY_TYPE {
		PLY_NULL = 0,
		PLY_FLOAT,
		PLY_INTEGER,
		PLY_LIST,
	};

	class PlyProperty
	{
	public:
		PlyProperty()
			: next(nullptr)
			, size(0)
			, type(PLY_NULL)
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
				delete to_del;
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

	class CPlyFile
	{
	public:
		CPlyFile(const std::string &file_path);
		~CPlyFile();
		
		// read header
		static void read_header(std::ostream &out, const std::string &file_path);
		void detail(std::ostream &out) const;

		// read vertex
		inline unsigned vertex_count() const {
			auto *elem = _find_element("vertex");
			return elem ? elem->count : 0;

		}
		inline bool has_position() const {
			return _find_vector3(PLY_FLOAT, "x", "y", "z");
		}
		inline bool has_normal() const {
			return _find_vector3(PLY_FLOAT, "nx", "ny", "nz");
		}
		inline bool has_color() const {
			return _find_vector3(PLY_INTEGER, "red", "green", "blue");
		}
		inline bool read_position(float *vector3, unsigned &count, unsigned stride) {
			return _read_vector3(vector3, count, stride, "x", "y", "z");
		}
		inline bool read_normal(float *vector3, unsigned &count, unsigned stride) {
			return _read_vector3(vector3, count, stride, "nx", "ny", "nz");
		}
		inline bool read_color(float *vector3, unsigned &count, unsigned stride) {
			return _read_vector3(vector3, count, stride, "red", "green", "blue");
		}
		
		// read triangles
		bool read_face(unsigned *vertex_indices, unsigned &count);

		// error handling
		inline operator bool() const {
			return m_error == PLY_NO_ERROR;
		}
		inline PLY_ERROR get_error() const {
			return m_error;
		}
		const char* get_error_desc() const;

	private:
		void _clear();
		bool _load(const std::string &file_path);
		PlyElement* _locate_element(const char *elem_name);
		std::streamoff _calculate_chunk_size(PlyElement *elem, std::streampos pos);
		const PlyElement* _find_element(const char *elem_name) const;
		bool _read_vector3(float *vector3, unsigned &count, unsigned stride, const char *v1, const char *v2, const char *v3);
		bool _find_vector3(PLY_PROPERTY_TYPE type, const char *v1, const char *v2, const char *v3) const;
		
		std::ifstream m_stream;
		std::streampos m_data_pos;
		PLY_ERROR m_error;
		PlyElement *m_elements;
		bool m_is_binary;
		bool m_is_little_endian;
	};

} // namespace wyc