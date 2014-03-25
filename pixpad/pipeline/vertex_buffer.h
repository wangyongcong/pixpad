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

	template<typename VERTEX> struct VERTEX_META_DATA {};

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
		inline void* get_data() {
			return m_data;
		}
		inline const vertex_t& operator[] (size_t idx) const {
			return m_data[idx];
		}
		inline vertex_t& operator[](size_t idx) {
			return m_data[idx];
		}
		template<VERTEX_ATTRIBUTE T>
		inline void* get_attr() {
			return m_data + VERTEX_META_DATA<VERTEX>::offset<T>();
		}

	private:
		vertex_t *m_data;
		size_t m_size;
	};

#define META_ATTR(name,attr_name) \
	typedef decltype(vertex_t::name) name##_t;\
	typedef name##_t::element_t name##_comp_t;\
	enum { name##_component=name##_t::DIMENSION };\
	template<> static inline size_t offset<attr_name>() {\
		return offsetof(vertex_t, name);\
	}
#define META_BEG(class_name) \
	template<> struct VERTEX_META_DATA<class_name> {\
		typedef class_name vertex_t;\
		template<VERTEX_ATTRIBUTE T> static inline size_t offset();

#define META_END };

	struct VERTEX_P3C3
	{
		xvec3f_t position;
		xvec3f_t color;
	};
	META_BEG(VERTEX_P3C3)
		META_ATTR(position, VERTEX_POSITION)
		META_ATTR(color, VERTEX_COLOR)
	META_END
	
}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER