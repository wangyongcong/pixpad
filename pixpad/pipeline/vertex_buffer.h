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
	};

#define VTID(type_name) TID_##type_name

	enum VERTEX_TID
	{
		VTID(VERTEX_P3),
		VTID(VERTEX_P3C3),
		VTID(VERTEX_P3_UV),
	};

#define VERTEX_HEADER(type_name) \
	enum { TID = VTID(type_name) };

	struct VERTEX_P3
	{
		VERTEX_HEADER(VERTEX_P3)
		xvec3f_t position;
	};

	struct VERTEX_P3C3
	{
		VERTEX_HEADER(VERTEX_P3C3)
		xvec3f_t position;
		xvec3f_t color;
	};

	struct VERTEX_P3_UV
	{
		VERTEX_HEADER(VERTEX_P3_UV)
		xvec3f_t position;
		xvec2f_t uv;
	};

	class xvertex_buffer
	{
	public:
		xvertex_buffer()
		{
			m_data = 0;
			m_size = 0;
		}
		~xvertex_buffer()
		{
			release();
		}
		template<typename VERTEX>
		bool storage(size_t vertex_count)
		{
			if (m_data) 
				release();
			m_vtid = VERTEX::TID;
			m_data = new VERTEX[vertex_count];
			m_size = vertex_count;
			m_stride = sizeof(VERTEX);
			return true;
		}
		void release()
		{
			if (!m_data) return;
			delete[] m_data;
			m_data = 0;
			m_size = 0;
		}
		void* get_data() {
			return m_data;
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
		VERTEX_TID vertex_type() const 
		{
			return m_vtid;
		}
	private:
		VERTEX_TID m_vtid;
		size_t m_size;
		size_t m_stride;
		void *m_data;
	};

	class xindex_buffer
	{
	public:
		xindex_buffer()
		{
			m_data = 0;
			m_size = 0;
		}
		~xindex_buffer()
		{
			release();
		}
		bool storage(size_t index_count, size_t vertex_count = 0)
		{
			if (m_data) 
				release();
			if (!vertex_count || vertex_count > 65535) {
				m_data = new uint32_t[index_count];
				m_stride = sizeof(uint32_t);
			}
			else {
				m_data = new uint16_t[index_count];
				m_stride = sizeof(uint16_t);
			}
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
		size_t operator[] (size_t idx) const {
			if (m_stride == sizeof(uint32_t))
				return ((uint32_t*)m_data)[idx];
			return ((uint16_t*)m_data)[idx];
		}
		size_t size() const {
			return m_size;
		}
	private:
		void *m_data;
		size_t m_size;
		size_t m_stride;
	};

	template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
	void gen_plane(float w, float h, VERTEX_BUFFER &vertices, INDEX_BUFFER &indices);

//
// Template implementations
//

	template<typename VERTEX_BUFFER, typename INDEX_BUFFER>
	void gen_plane(float w, float h, VERTEX_BUFFER &vertices, INDEX_BUFFER &indices)
	{
		vertices.resize(4);
		indices.resize(6);
		float x = w*0.5f;
		float y = h*0.5f;
		vertices[0].position.set(-x, -y, 0);
		vertices[1].position.set(x, -y, 0);
		vertices[2].position.set(x, y, 0);
		vertices[3].position.set(-x, y, 0);
		indices[0] = 0; indices[1] = 1; indices[2] = 3;
		indices[3] = 3; indices[4] = 1; indices[5] = 2;
	}

}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER