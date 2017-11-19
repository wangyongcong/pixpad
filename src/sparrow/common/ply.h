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
		PLY_UNKNOWN_ELEMENT,
		PLY_UNKNOWN_PROPERTY,
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
		static void read_header(std::ostream &out, const std::string &file_path);

		CPlyFile(const std::string &file_path);
		~CPlyFile();
		void detail(std::ostream &out) const;
		const PlyElement* find_element(const std::string &name);
		bool read_vertex_position(float *vector3, unsigned &count, unsigned stride);
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
		
		std::ifstream m_stream;
		std::streampos m_data_pos;
		PLY_ERROR m_error;
		PlyElement *m_elements;
		bool m_is_binary;
		bool m_is_little_endian;
	};

} // namespace wyc