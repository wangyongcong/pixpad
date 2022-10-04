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

	class PlyElement;

	class CPlyFile
	{
	public:
		CPlyFile(const std::string &file_path);
		~CPlyFile();
		
		// read header
		static void read_header(std::ostream &out, const std::string &file_path);
		void detail(std::ostream &out) const;

		// read vertex
		unsigned vertex_count() const;
		bool has_position() const
		{
			return _find_vector3(PLY_FLOAT, "x", "y", "z");
		}
		bool has_normal() const
		{
			return _find_vector3(PLY_FLOAT, "nx", "ny", "nz");
		}
		bool has_color() const
		{
			return _find_vector3(PLY_INTEGER, "red", "green", "blue");
		}
		bool read_position(float *vector3, unsigned &count, unsigned stride)
		{
			return _read_vector3(vector3, count, stride, "x", "y", "z");
		}
		bool read_normal(float *vector3, unsigned &count, unsigned stride)
		{
			return _read_vector3(vector3, count, stride, "nx", "ny", "nz");
		}
		bool read_color(float *vector3, unsigned &count, unsigned stride)
		{
			return _read_vector3(vector3, count, stride, "red", "green", "blue");
		}
		
		// read vertex
		bool read_vertex(float *vertex, unsigned &count, const std::string &layout, unsigned stride);
		// read triangles
		bool read_face(unsigned *vertex_indices, unsigned &count);

		// error handling
		operator bool() const
		{
			return m_error == PLY_NO_ERROR;
		}
		PLY_ERROR get_error() const
		{
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
