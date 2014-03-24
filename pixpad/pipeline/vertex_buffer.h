#ifndef WYC_HEADER_VERTEX_BUFFER
#define WYC_HEADER_VERTEX_BUFFER

#include <cstddef>
#include "mathex/vecmath.h"

namespace wyc
{

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
		}
		void release() {
			delete[] m_data;
			m_data = 0;
			m_size = 0;
		}
		inline const vertex_t& operator[] (size_t idx) const {
			return m_data[idx];
		}
		inline vertex_t& operatorp[](size_t idx) {
			return m_data[idx];
		}
	private:
		vertex_t *m_data;
		size_t m_size;
	};

	struct VERTEX_P3C3
	{
		xvec3f_t position;
		xvec3f_t color;

		// data type
		typedef float POSITION_DATA_TYPE;
		typedef float COLOR_DATA_TYPE;
		// component count
		enum COMPONENT {
			POSITION_COMPONENT = 3,
			COLOR_COMPONENT = 3,
		};
	};

	template<> class xvertex_buffer<VERTEX_P3C3>
	{
	};

}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER