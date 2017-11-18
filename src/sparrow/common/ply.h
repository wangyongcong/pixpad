#pragma once
#include <string>

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

	class PlyProperty
	{
	public:
		PlyProperty()
			: size(0)
			, type(PLY_NULL)
			, next(nullptr)
		{
		}
		std::string name;
		PLY_PROPERTY_TYPE type;
		unsigned size;
		PlyProperty *next;
	};

	class PlyElement
	{
	public:
		PlyElement()
			: count(0)
			, size(0)
			, properties(nullptr)
			, next(nullptr)
			, offset(0)
			, readers(nullptr)
		{
		}
		~PlyElement()
		{
		}
		std::string name;
		unsigned count;
		unsigned size;
		PlyProperty *properties;
		PlyElement *next;
		std::streampos offset;
		IPlyReader *readers;
		bool is_variant;
	};

	class CPlyFile
	{
	public:
		static void read_header(std::ostream &out, const std::string &file_path);

		CPlyFile(const std::string &file_path);
		~CPlyFile();
		inline operator bool() const {
			return m_error == PLY_NO_ERROR;
		}
		inline PLY_ERROR get_error() const {
			return m_error;
		}
		const char* get_error_desc() const;
		void detail(std::ostream &out) const;

	private:
		bool _load(const std::string &file_path);
		bool _read_binary_le(std::istream &fin, std::streampos pos);
		bool _read_binary_be(std::istream &fin);
		bool _read_ascii(std::istream &fin);
		void _read_vertex(std::istream &fin, PlyElement *elem);
		
		void _clear();
		PLY_ERROR m_error;
		PlyElement *m_elements;
	};

} // namespace wyc