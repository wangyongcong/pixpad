#ifndef WYC_HEADER_VERTEX_BUFFER
#define WYC_HEADER_VERTEX_BUFFER

#include <cstddef>
#include "mathex/vecmath.h"

namespace wyc
{
	enum VERTEX_ATTRIBUTE {
		VERTEX_POSITION,
		VERTEX_COLOR,
		VERTEX_NORMAL,
	};

	template<typename VERTEX>
	class xvertex_buffer
	{
	public:
		typedef typename VERTEX vertex_t;
		enum { STRIDE = sizeof(vertex_t) };
		xvertex_buffer() {
			m_data = 0;
		}
		~xvertex_buffer() {
			if (m_data) {
				delete[] m_data;
				m_data = 0;
			}
		}
		// copy is not allowable
		xvertex_buffer(const xvertex_buffer&) = delete;
		xvertex_buffer& operator = (const xvertex_buffer&) = delete;

		bool storage(size_t vertex_count){
			if (vertex_count == 0)
				return false;
			if (m_data) release();
			m_data = new vertex_t[vertex_count];
			m_size = vertex_count;
			return true;
		}
		void release() {
			delete[] m_data;
			m_data = 0;
			m_size = 0;
		}
		inline size_t size() const {
			return m_size;
		}
		inline size_t size_in_bytes() const {
			return m_size * sizeof(vertex_t);
		}
		inline void* get_data() {
			return m_data;
		}
		inline const vertex_t& operator[] (size_t idx) const {
			return m_data[idx];
		}
		inline vertex_t& operator[](size_t idx) {
			return m_data[idx];
		}

		// Get attribute array pointer
		template<VERTEX_ATTRIBUTE T>
		void* get_ptr();

#define GET_PTR_IMPL(m,attr) template<> inline void* get_ptr<attr>() {return &m_data->m;}

		GET_PTR_IMPL(position, VERTEX_POSITION)
		GET_PTR_IMPL(color, VERTEX_COLOR);

	private:
		vertex_t *m_data;
		size_t m_size;
	};

	struct VERTEX_P3C3
	{
		xvec3f_t position;
		xvec3f_t color;
	};
		
}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER