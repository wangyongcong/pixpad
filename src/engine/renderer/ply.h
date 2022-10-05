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

	enum PLY_PROPERTY_TYPE
	{
		PLY_NULL = 0,
		PLY_FLOAT,
		PLY_INTEGER,
		PLY_LIST,
	};

	class PlyElement;
	class PlyProperty;

	class CPlyFile
	{
	public:
		CPlyFile(const std::string &file_path);
		~CPlyFile();
		
		// read header
		static void read_header(std::ostream &out, const std::string &file_path);
		void detail(std::ostream &out) const;

		// get vertex count
		unsigned vertex_count() const;
		// get face (triangle) count
		unsigned face_count();

		// read vertex
		bool read_vertex(float *vertex, unsigned &count, const std::string &layout, unsigned stride);
		bool read_vertex(char *buffer, size_t buffer_size);
		// read triangles
		bool read_face(char *buffer, size_t buffer_size, unsigned stride);

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

		class PropertyIterator
		{
		public:
			PropertyIterator(const PlyProperty*);
			PropertyIterator& operator++ ();
			PropertyIterator operator++ (int);
			PropertyIterator& operator+= (int);
			operator bool() const { return m_property != nullptr; }
			bool is_float() const;
			bool is_integer() const;
			unsigned size() const;
			const std::string& name() const;
			bool is_vector(const char* x, const char* y, const char* z) const;
		private:
			const PlyProperty* m_property;
		};
		PropertyIterator get_vertex_property() const;

	private:
		void _clear();
		bool _load(const std::string &file_path);
		PlyElement* _locate_element(const char *elem_name);
		std::streamoff _calculate_chunk_size(PlyElement *elem, std::streampos pos);
		const PlyElement* _find_element(const char *elem_name) const;
		bool _read_vector3(float *vector3, unsigned &count, unsigned stride, const char *v1, const char *v2, const char *v3);
		bool _find_vector3(PLY_PROPERTY_TYPE type, const char *v1, const char *v2, const char *v3) const;
		bool _read_face(char *buffer, size_t buffer_size, unsigned stride, unsigned &count);

		std::ifstream m_stream;
		std::streampos m_data_pos;
		PLY_ERROR m_error;
		PlyElement *m_elements;
		unsigned m_face_count;

		bool m_is_binary;
		bool m_is_little_endian;
		bool m_is_face_counted;
	};

} // namespace wyc
