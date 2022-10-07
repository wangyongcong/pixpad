#pragma once
#include <string>
#include <fstream>
#include "common/util_macros.h"

namespace wyc
{
	enum EPlyError : uint8_t
	{
		PLY_NO_ERROR = 0,
		PLY_FILE_NOT_FOUND,
		PLY_INVALID_FILE,
		PLY_UNKNOWN_FORMAT,
		PLY_INVALID_PROPERTY,
		PLY_NOT_SUPPORT_ASCII,
		PLY_NOT_SUPPORT_BID_ENDIAN,
		PLY_MULTIPLE_LIST_PROPERTY,
		PLY_LIST_LENGTH_OVERFLOW,
		PLY_LIST_INVALID_ITEM_SIZE,
		PLY_IO_INTERRUPT,
		PLY_INVALID_FACE_LIST,
	};

	enum EPlyPropertyType : uint8_t
	{
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
			, type(PLY_NULL)
			, size(0)
		{
		}

		PlyProperty *next;
		std::string name;
		EPlyPropertyType type;
		union
		{
			// property data size in byte for non-list property
			uint32_t size;
			struct
			{
				// list length size in byte
				uint8_t length_size;
				// list item size in byte
				uint8_t item_size;
				// list item data type
				EPlyPropertyType item_type;
			};
		};

		bool is_vector(const char* x, const char* y, const char* z, PlyProperty const** last) const;
		bool is_float() const
		{
			return type == PLY_FLOAT;
		}
		bool is_integer() const
		{
			return type == PLY_INTEGER;
		}
	};

	class PlyElement
	{
	public:
		PlyElement()
			: next(nullptr)
			, count(0)
			, size(0)
			, properties(nullptr)
			, property_count(0)
		{
		}

		PlyElement *next;
		std::string name;
		// element count
		unsigned count;
		// it's the size of all properties
		unsigned size;
		PlyProperty *properties;
		unsigned property_count;
	};

	class PlyElementImpl;
	class PlyPropertyImpl;

	class CPlyFile
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(CPlyFile)
	public:
		CPlyFile(const std::string &file_path);
		~CPlyFile();
		// read header
		static void read_header(std::ostream &out, const std::string &file_path);
		// print element property info
		void detail(std::ostream &out) const;
		void detail() const;
		/**
		 * \brief Load geometry data
		 * \return is successful
		 */
		bool load();
		// get vertex count
		unsigned vertex_count() const;
		// get face (triangle) count
		unsigned face_count() const;
		/**
		 * \brief Read vertex data
		 * \param buffer Buffer to save vertex data
		 * \param buffer_size Buffer size in byte
		 * \return Number of vertex were read
		 */
		unsigned read_vertex(char* buffer, size_t buffer_size) const;
		/**
		 * \brief Read vertex indices
		 * \param buffer Buffer to save indices data
		 * \param buffer_size Buffer size in byte
		 * \param index_size Size of each index (in byte)
		 * \return Number of index were read
		 */
		unsigned read_face(char *buffer, size_t buffer_size, unsigned index_size) const;
		/**
		 * \brief Find element info by name
		 * \param elem_name element name
		 * \return ply element info
		 */
		const PlyElement* find_element(const char *elem_name) const;
		// get vertex property
		const PlyProperty* get_vertex_property() const;
		// error handling
		operator bool() const
		{
			return m_error == PLY_NO_ERROR;
		}
		EPlyError get_error() const
		{
			return m_error;
		}
		const char* get_error_desc() const;

	private:
		void clear();
		bool preload(const std::string &file_path);

		std::string m_path;
		std::ifstream m_stream;
		std::streampos m_data_pos;
		EPlyError m_error;
		PlyElement *m_elements;
		mutable unsigned m_cached_face_count;

		bool m_is_binary;
		bool m_is_little_endian;
		bool m_is_data_loaded;
	};

} // namespace wyc
