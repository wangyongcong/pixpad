#ifndef WYC_HEADER_VERTEX_BUFFER
#define WYC_HEADER_VERTEX_BUFFER

#include <cassert>
#include <cstddef>
#include <vector>
#include "mathex/vecmath.h"

namespace wyc
{

	enum VERTEX_ATTRIBUTE
	{
		VERTEX_POSITION = 0,
		VERTEX_COLOR,
		VERTEX_NORMAL,
		VERTEX_TANGENT,
		VERTEX_UV,

		VERTEX_ATTRIBUTE_COUNT,
	};

	enum DATA_TYPE
	{
		BYTE,
		UBYTE,
		SHORT,
		USHORT,
		INT,
		UINT,
		FLOAT,
		DOUBLE,
	};

	template<DATA_TYPE T>
	struct ctype;
	template<> struct ctype<BYTE>
	{
		typedef int8_t value_t;
	};
	template<> struct ctype<UBYTE>
	{
		typedef uint8_t value_t;
	};
	template<> struct ctype<SHORT>
	{
		typedef int16_t value_t;
	};
	template<> struct ctype<USHORT>
	{
		typedef uint16_t value_t;
	};
	template<> struct ctype<INT>
	{
		typedef int32_t value_t;
	};
	template<> struct ctype<UINT>
	{
		typedef uint32_t value_t;
	};


	struct xvertex_attrib
	{
		VERTEX_ATTRIBUTE attrib_type;
		uint8_t component;
		DATA_TYPE data_type;
		size_t offset;
	};

	struct vertex_p3
	{
		xvec3f_t position;
	};

	struct vertex_p3c3
	{
		xvec3f_t position;
		xvec3f_t color;
	};

	struct vertex_p3uv
	{
		xvec3f_t position;
		xvec2f_t uv;
	};

	template<typename VERTEX> 
	const xvertex_attrib* vertex_layout(unsigned &count);

	template<> const xvertex_attrib* vertex_layout<vertex_p3>(unsigned &count)
	{
		static xvertex_attrib _layout[1] = {
			{ VERTEX_POSITION, 3, FLOAT, offsetof(vertex_p3, position) },
		};
		count = 1;
		return _layout;
	}

	template<> const xvertex_attrib* vertex_layout<vertex_p3c3>(unsigned &count)
	{
		static xvertex_attrib _layout[2] = {
			{ VERTEX_POSITION, 3, FLOAT, offsetof(vertex_p3c3, position) },
			{ VERTEX_COLOR, 3, FLOAT, offsetof(vertex_p3c3, color) },
		};
		count = 2;
		return _layout;
	}

	class xvertex_buffer
	{
	public:
		xvertex_buffer()
		{
			m_data = 0;
			m_size = 0;
			m_stride = 0;
			m_attribs = 0;
			m_attrib_cnt = 0;
		}
		~xvertex_buffer()
		{
			release();
		}
		template<typename VERTEX>
		bool storage(size_t vertex_count)
		{
			if (m_data) 
				delete[] m_data;
			m_data = new VERTEX[vertex_count];
			m_size = vertex_count;
			m_stride = sizeof(VERTEX);
			m_attribs = vertex_layout<VERTEX>(m_attrib_cnt);
			return true;
		}
		void release()
		{
			if (!m_data) return;
			delete[] m_data;
			m_data = 0;
			m_size = 0;
			m_stride = 0;
			m_attribs = 0;
			m_attrib_cnt = 0;
		}
		const void* get_data() const {
			return m_data;
		}
		size_t size() const
		{
			return m_size;
		}
		size_t size_in_bytes() const
		{
			return m_size*m_stride;
		}
		const xvertex_attrib* get_attribs() const
		{
			return m_attribs;
		}
		unsigned attrib_count() const
		{
			return m_attrib_cnt;
		}
		template<typename VERTEX>
		VERTEX* get_as() {
			unsigned count;
			if (vertex_layout<VERTEX>(count) != m_attribs) return 0;
			return reinterpret_cast<VERTEX*>(m_data);
		}
		template<typename VERTEX>
		const VERTEX* get_as() const {
			unsigned count;
			if (vertex_layout<VERTEX>(count) != m_attribs) return 0;
			return reinterpret_cast<VERTEX*>(m_data);
		}
	private:
		void *m_data;
		size_t m_size;
		size_t m_stride;
		const xvertex_attrib *m_attribs;
		unsigned m_attrib_cnt;
	};

	class xindex_buffer
	{
	public:
		xindex_buffer()
		{
			m_data = 0;
			m_size = 0;
			m_type = UINT;
		}
		~xindex_buffer()
		{
			release();
		}
		template<DATA_TYPE T>
		bool storage(size_t index_count)
		{
			if (m_data)
				delete[] m_data;
			m_data = new ctype<T>::value_t[index_count];
			m_type = T;
			m_size = index_count;
			return true;
		}
		void release()
		{
			if (!m_data) return;
			delete[] m_data;
			m_data = 0;
			m_size = 0;
		}
		const void* get_data() const {
			return m_data;
		}
		template<DATA_TYPE T>
		typename ctype<T>::value_t* get_as() {
			if (T != m_type) return 0;
			return reinterpret_cast<ctype<T>::value_t*>(m_data);
		}
		template<DATA_TYPE T>
		const typename ctype<T>::value_t* get_as() const {
			if (T != m_type) return 0;
			return reinterpret_cast<ctype<T>::value_t*>(m_data);
		}
		size_t size() const {
			return m_size;
		}
		DATA_TYPE index_type() const {
			return m_type;
		}
	private:
		void *m_data;
		size_t m_size;
		DATA_TYPE m_type;
	};

	template<typename VERTEX>
	void gen_plane(float w, float h, xvertex_buffer &vertices, xindex_buffer &indices);

//
// Template implementations
//

	template<typename VERTEX>
	void gen_plane(float w, float h, xvertex_buffer &vertices, xindex_buffer &indices)
	{
		vertices.storage<VERTEX>(4);
		indices.storage<UBYTE>(6);
		float x = w*0.5f;
		float y = h*0.5f;
		VERTEX *v = vertices.get_as<VERTEX>();
		v[0].position.set(-x, -y, 0);
		v[1].position.set(x, -y, 0);
		v[2].position.set(x, y, 0);
		v[3].position.set(-x, y, 0);
		auto i = indices.get_as<UBYTE>();
		i[0] = 0; i[1] = 1; i[2] = 3;
		i[3] = 3; i[4] = 1; i[5] = 2;
	}

}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER